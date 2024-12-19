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
#include "Gaudi/Property.h"
#include "k4FWCore/Transformer.h"
#include <edm4hep/ParticleIDCollection.h>
#include <edm4hep/ReconstructedParticleCollection.h>
#include <edm4hep/VertexCollection.h>

#include <nlohmann/json.hpp> // Include a JSON parsing library
#include <fstream>

#include "Structs.h"
#include "JetObservablesRetriever.h"
#include "VarMapper.h"
#include "WeaverInterface.h"

nlohmann::json loadJsonFile(const std::string& json_path) {
  /**
  * Load a JSON file from a given path.
  * @param json_path: the path to the JSON file
  * @return: the JSON object
  */
  std::ifstream json_file(json_path);
  if (!json_file.is_open()) {
    std::cerr << "Failed to open JSON file: " << json_path << std::endl;
    return nlohmann::json();
  }
  nlohmann::json json_config;
  json_file >> json_config;
  return json_config;
}


bool check_flavors(std::vector<std::string>& flavorNames, const std::vector<std::string>& flavor_collection_names) {
  /** Check if the flavor names from the JSON configuration file match the flavor collection names in the python config file
  * @param flavorNames: the flavor names from the JSON configuration file
  * @param flavor_collection_names: the flavor collection names from the python config file
  * @return: true if the flavor names match the flavor collection names, false otherwise
  */

  if (flavorNames.size() != flavor_collection_names.size()) {
    std::cerr << "The number of flavors in the JSON configuration file does not match the number of flavor collection names in the python config file." << std::endl;
    return false;
  }

  for (size_t i = 0; i < flavorNames.size(); ++i) {
    // Extract the flavor identifier from the flavor name
    std::string flavorName = flavorNames[i];
    std::string flavorCollectionName = flavor_collection_names[i];

    // Find the flavor identifier by locating the last underscore
    size_t flavorNamePos = flavorName.find_last_of('_');
    size_t flavorCollectionPos = flavorCollectionName.find_last_of('_');

    if (flavorNamePos == std::string::npos || flavorCollectionPos == std::string::npos) {
      std::cerr << "Invalid format for flavor names or collection names at index " << i << "." << std::endl;
      return false;
    };

    std::string extractedFlavorName = flavorName.substr(flavorNamePos + 3); // +3 to remove the "is" prefix
    std::string extractedFlavorCollectionName = flavorCollectionName.substr(flavorCollectionPos + 1);

    // Compare the extracted flavors
    if (extractedFlavorName != extractedFlavorCollectionName) {
      std::cerr << "Mismatch at index " << i << ": Flavor name (" << extractedFlavorName
                << ") does not match flavor collection name (" << extractedFlavorCollectionName << ")." << std::endl;
      return false;
    };
  }

  // If all checks pass
  return true;
}

rv::RVec<rv::RVec<float>> from_Jet_to_onnx_input(Jet& jet, rv::RVec<std::string>& input_names){
  /**
  Return the input variables for the ONNX model from a Jet object. The input variables should have the form of {jet -> {var1 -> {constit1, constit2, ...}, var2 -> {...}, ...}}
  * @param jet: the jet object
  * @param input_names: the names of the input variables for the ONNX model.
  * @return: the input variables for the ONNX model
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

  // change from {constituent -> {var1, var2, ...}} to {var1 -> {constituent1, constituent2, ...}, var2 -> {...}, ...}
  rv::RVec<rv::RVec<float>> input_vars;
  for (unsigned int i = 0; i < constituent_vars[0].size(); i++) { // loop over all variables
    rv::RVec<float> var;
    for (unsigned int j = 0; j < constituent_vars.size(); j++) { // loop over all constituents
      var.push_back(constituent_vars[j][i]);
    }
    input_vars.push_back(var);
  }

  return input_vars;

};


rv::RVec<float> tagger(Jet& jet, const std::string& model_path, const std::string& json_path) {
  /**
  * Taggs a jet. The function loads the ONNX model and the JSON configuration file from hardcoded paths, retrieves the input variables for the ONNX model from the Jet object, runs inference on the input variables and returns the probabilities for each jet flavor.
  * @param jet: the jet object to tag
  * @param model_path: the path to the ONNX model
  * @param json_path: the path to the JSON configuration file of the ONNX model
  * @return: tuple of two elements: 1. the probabilities for each jet flavor, 2. the names of the jet flavors
  */
  // Load json configuration file
  nlohmann::json json_config = loadJsonFile(json_path);

  // retrieve the input variable to onnx model from json file
  rv::RVec<std::string> vars; // e.g. pfcand_isEl, ...
  for (const auto& var : json_config["pf_features"]["var_names"]) {
    vars.push_back(var.get<std::string>());
  }
  for (const auto& var : json_config["pf_vectors"]["var_names"]) { // not sure if this is the solution here
    vars.push_back(var.get<std::string>());
  }
  // variables in pf_points are already included in pf_features

  // Create the WeaverInterface object
  WeaverInterface weaver(model_path, json_path, vars);

  // Convert the Jet object to the input format for the ONNX model
  rv::RVec<rv::RVec<float>> jet_const_data = from_Jet_to_onnx_input(jet, vars);

  // Run inference on the input variables - returns the 7 probabilities for each jet flavor
  rv::RVec<float> probabilities = weaver.run(jet_const_data);

  // print results
  // retrieve output variable names from json file
  // rv::RVec<std::string> output_names; // e.g. "recojet_isX" with X being the jet flavor (G, U, S, C, B, D, TAU)
  // for (const auto& var : json_config["output_names"]) {
  //   output_names.push_back(var.get<std::string>());
  // }
  // std::cout << "------------------------" << std::endl;
  // for (unsigned int i = 0; i < probabilities.size(); i++) {
  //   std::cout << "Probability for jet flavor " << output_names[i] << ": " << probabilities[i] << std::endl;
  // }

  return probabilities;
}

// main function
struct JetTagger
    : k4FWCore::Transformer<std::vector<edm4hep::ParticleIDCollection>(const edm4hep::ReconstructedParticleCollection&, const edm4hep::VertexCollection& )> {
  JetTagger(const std::string& name, ISvcLocator* svcLoc)
    : Transformer(name, svcLoc, 
                  {
                    KeyValues("InputJets", {"RefinedVertexJets"}),
                    KeyValues("InputPrimaryVertices", {"PrimaryVertices"})
                  },
                  {KeyValues("OutputIDCollections", {"RefinedJetTags"})}
                  ) {}

  std::vector<edm4hep::ParticleIDCollection> operator()(const edm4hep::ReconstructedParticleCollection& inputJets, const edm4hep::VertexCollection& primVerticies) const override{
    info() << "Tagging " << inputJets.size() << " input jets" << endmsg;
    // Load JSON configuration to retrieve how many flavors are tagged
    nlohmann::json json_config = loadJsonFile(json_path);
    std::vector<std::string> flavorNames = json_config["output_names"]; // e.g. "recojet_isX" with X being the jet flavor (G, U, S, C, B, D, TAU)

    // check if flavorNames matches order and size of the output collections
    if (!check_flavors(flavorNames, flavor_collection_names)) {
      error() << "ATTENTION! Output flavor collection names MUST match ONNX model output flavors!" << endmsg;
    }
    
    // create n ParticleIDCollection objects, one for each flavor
    std::vector<edm4hep::ParticleIDCollection> tagCollections;
    for (const auto& flavor : flavorNames) {
      tagCollections.push_back(edm4hep::ParticleIDCollection());
    }

    JetObservablesRetriever Retriever;

    // DUMMY 
    auto tagCollection = edm4hep::ParticleIDCollection();

    for (const auto& jet : inputJets) {
      Jet j = Retriever.retrieve_input_observables(jet, primVerticies);
      rv::RVec<float> probabilities = tagger(j, model_path, json_path);

      if (probabilities.size() != flavorNames.size()) {
        error() << "Number of probabilities returned by the network does not match number of flavors stated in the network config json" << endmsg;
      }

      for (unsigned int i = 0; i < flavorNames.size(); i++) {
        auto jetTag = tagCollections[i].create();
        jetTag.setParticle(jet);
        jetTag.setLikelihood(probabilities[i]);
        // jetTag.setType(i);
      }
      // int tagValue = 0; 
      // // Handle tag collection
      // auto jetTag = tagCollection.create();
      // jetTag.setParticle(jet);
      // jetTag.setType(tagValue); // maybe alter this
    }

    return tagCollections;
  };

  private: 
    Gaudi::Property<std::string> model_path{
      this,
      "model_path",
      "/afs/cern.ch/work/s/saaumill/public/onnx_export/fullsimCLD240_2mio.onnx", 
      "Path to the ONNX model"
    };
    Gaudi::Property<std::string> json_path{
      this, 
      "json_path",
      "/afs/cern.ch/work/s/saaumill/public/onnx_export/preprocess_fullsimCLD240_2mio.json",
      "Path to the JSON configuration file for the ONNX model"
    };
    Gaudi::Property<std::vector<std::string>> flavor_collection_names{
      this,
      "flavor_collection_names",
      {"RefinedJetTag_G", "RefinedJetTag_U", "RefinedJetTag_S", "RefinedJetTag_C", "RefinedJetTag_B", "RefinedJetTag_D", "RefinedJetTag_TAU"},
      "Names of the output collections"
    };
};

DECLARE_COMPONENT(JetTagger)
