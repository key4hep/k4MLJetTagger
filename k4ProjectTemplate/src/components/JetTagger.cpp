/*
 * Copyright (c) 2020-2024 Key4hep-Project.
 *
 * This file is part of Key4hep.
 * See https://key4hep.github.io/key4hep-doc/ for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "GaudiKernel/MsgStream.h"
#include "k4FWCore/Transformer.h"
#include <edm4hep/ParticleIDCollection.h>
#include <edm4hep/ReconstructedParticleCollection.h>
#include <edm4hep/VertexCollection.h>


#include <nlohmann/json.hpp> // Include a JSON parsing library
#include <fstream>

#include <random>
#include "VarMapper.h"
#include "WeaverInterface.h"
#include "Structs.h"
#include "JetObservablesRetriever.h"



// helper functions to run inference on the neural network

rv::RVec<rv::RVec<float>> from_Jet_to_onnx_input(Jet& jet, rv::RVec<std::string>& input_names){
  /**
  Return the input variables for the ONNX model from a Jet object. The input variables should have the form of {jet -> {var1 -> {constit1, constit2, ...}, var2 -> {...}, ...}}
  * @param jet: the jet object
  * @param input_names: the names of the input variables for the ONNX model.
  * @return: the input variables for the ONNX model
  * NOTE: the order of the variables must match the order of the input variables to the neural network
  */
  rv::RVec<rv::RVec<float>> constituent_vars;
  VarMapper mapper; // transform the names of the variables (ONNX (aka FCCAnalyses) convention <-> key4hep convention)
  for (auto& pfcand : jet.constituents) { // loop over all constituents
    rv::RVec<float> vars;
    // loop over all expected input observables
    for (auto& obs : input_names){
      std::string key4hepName = mapper.mapFCCAnToKey4hep(obs); // map the variable name to the key4hep convention
      vars.push_back(pfcand.get_attribute(key4hepName));
    }
    // add the vars to the constituent_vars
    constituent_vars.push_back(vars);
  }
  std::cout << "Number of constituents: " << constituent_vars.size() << std::endl;

  // change from {constituent -> {var1, var2, ...}} to {var1 -> {constituent1, constituent2, ...}, var2 -> {...}, ...}
  rv::RVec<rv::RVec<float>> input_vars;
  for (int i = 0; i < constituent_vars[0].size(); i++) { // loop over all variables
    rv::RVec<float> var;
    for (int j = 0; j < constituent_vars.size(); j++) { // loop over all constituents
      var.push_back(constituent_vars[j][i]);
    }
    input_vars.push_back(var);
  }

  return input_vars;

};

int tagger(Jet& jet){
  /**
  * Function that takes a jet and returns a tag value. This function is a dummy function that returns a random tag value for demonstration purposes.
  * @param jet: the jet to tag
  * @return: the tag value
  */
  // Create a random number generator for demonstration purposes.
  static std::mt19937 rng(42);  // Seed for reproducibility
  static std::uniform_int_distribution<int> dist(0, 6);  // Assume 7 jet flavors: 0 to 6

  // Generate a random tag value for the jet.
  int tagValue = dist(rng);

  // Path to your ONNX model
  const char* model_path = "/afs/cern.ch/work/s/saaumill/public/onnx_export/fullsimCLD240_2mio.onnx";
  const char* json_path = "/afs/cern.ch/work/s/saaumill/public/onnx_export/preprocess_fullsimCLD240_2mio.json";

  // Load JSON configuration
  std::ifstream json_file(json_path);
  if (!json_file.is_open()) {
    std::cerr << "Failed to open JSON file: " << json_path << std::endl;
    return -1;
  }
  nlohmann::json json_config;
  json_file >> json_config;

  std::cout << "Loaded JSON configuration" << std::endl;

  // retrieve the input variable to onnx model from json file
  rv::RVec<std::string> vars; // e.g. pfcand_isEl, ...
  for (const auto& var : json_config["pf_features"]["var_names"]) {
    vars.push_back(var.get<std::string>());
  }
  for (const auto& var : json_config["pf_vectors"]["var_names"]) { // not sure if this is the solution here
    vars.push_back(var.get<std::string>());
  }


  std::cout << "Loaded input variables" << std::endl;
  std::cout << "Number of input variables: " << vars.size() << std::endl;

  // Create the WeaverInterface object
  WeaverInterface weaver(model_path, json_path, vars);

  std::cout << "Created WeaverInterface object" << std::endl;

  // Convert the Jet object to the input format for the ONNX model
  rv::RVec<rv::RVec<float>> jet_const_data = from_Jet_to_onnx_input(jet, vars);

  std::cout << "Input data size: " << jet_const_data.size() << std::endl;


  // Run inference on the input variables for a list of jet constituents

  // NOTE: ERROR Unable to find variable with name 'pfcand_e' in the list of registered variables
  rv::RVec<float> probabilities = weaver.run(jet_const_data);

  // print results
  for (int i = 0; i < probabilities.size(); i++) {
    std::cout << "Probability for jet flavor " << i << ": " << probabilities[i] << std::endl;
  }


  return tagValue;
}


// main function
struct JetTagger
    : k4FWCore::Transformer<edm4hep::ParticleIDCollection(const edm4hep::ReconstructedParticleCollection&, const edm4hep::VertexCollection& )> {
  JetTagger(const std::string& name, ISvcLocator* svcLoc)
    : Transformer(name, svcLoc, 
                  {
                    KeyValues("InputJets", {"RefinedVertexJets"}),
                    KeyValues("InputPrimaryVertices", {"PrimaryVertices"})
                  },
                  {KeyValues("OutputIDCollections", {"RefinedJetTags"})}
                  ) {}

  edm4hep::ParticleIDCollection operator()(const edm4hep::ReconstructedParticleCollection& inputJets, const edm4hep::VertexCollection& primVerticies) const override{
    info() << "Tagging " << inputJets.size() << " input jets" << endmsg;
    auto tagCollection = edm4hep::ParticleIDCollection();

    JetObservablesRetriever Retriever;

    for (const auto& jet : inputJets) {
      Jet j = Retriever.retrieve_input_observables(jet, primVerticies);
      std::cout << "Jet constituents: " << j.constituents.size() << std::endl;
      int tagValue = tagger(j);  

      // Handle tag collection
      auto jetTag = tagCollection.create();
      jetTag.setParticle(jet);
      jetTag.setType(tagValue); // maybe alter this
    }

    return tagCollection;
  };
};

DECLARE_COMPONENT(JetTagger)
