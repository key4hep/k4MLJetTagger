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
#include "k4Interface/IGeoSvc.h" // for Bfield
#include <edm4hep/ParticleIDCollection.h>
#include <edm4hep/ReconstructedParticleCollection.h>
#include <edm4hep/VertexCollection.h>


#include <nlohmann/json.hpp> // Include a JSON parsing library
#include <fstream>

#include "Structs.h"
#include "JetObservablesRetriever.h"
#include "WeaverInterface.h"
#include "Helpers.h"


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

  return probabilities;
}

/**
* @class JetTagger
* @brief Gaudi transformer that builds edm4hep::ParticleIDCollection objects (!plural, one for each flavor) for each jet in edm4hep::ReconstructedParticleCollection. 
*
* We retrieve a description of the jet constituents which serve as an input to a neural network. The network is loaded as an ONNX model. 
* The inference is run on each jet. The output of the network is a vector of probabilities for each jet flavor.
* We create one ParticleID collection per flavor create, link it to the jet and set the likelihood and PDG number.
*
* @author Sara Aumiller
*/
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
      info() << "Flavors expected from network in this order: " << flavorNames << endmsg;
    }
    
    // create n ParticleIDCollection objects, one for each flavor & retrieve the PDG number for each flavor
    std::vector<edm4hep::ParticleIDCollection> tagCollections;
    std::vector<int> PDGflavors;
    for (const auto& flavor : flavorNames) {
      tagCollections.push_back(edm4hep::ParticleIDCollection());
      PDGflavors.push_back(to_PDGflavor.at(flavor)); // retrieve the PDG number from the flavor name
    }

    JetObservablesRetriever Retriever;
    // get B field from detector
    // dd4hep::Detector* theDetector = Gaudi::svcLocator()->service<IGeoSvc>("GeoSvc")->getDetector();
    // double Bfield = getBzAtOrigin(theDetector);
 
    Retriever.Bz = 2.0; // hardcoded for now

    for (const auto& jet : inputJets) {
      // retrieve the input observables to the network from the jet
      Jet j = Retriever.retrieve_input_observables(jet, primVerticies);
      // run the tagger on the jet
      rv::RVec<float> probabilities = tagger(j, model_path, json_path);

      if (probabilities.size() != flavorNames.size()) {
        error() << "Number of probabilities returned by the network does not match number of flavors stated in the network config json" << endmsg;
      }

      for (unsigned int i = 0; i < flavorNames.size(); i++) {
        auto jetTag = tagCollections[i].create();
        jetTag.setParticle(jet);
        jetTag.setLikelihood(probabilities[i]);
        jetTag.setPDG(PDGflavors[i]);
      }
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
      "Names of the output collections. Order, size and flavor labels _X must match the network configuration."
    };
};

DECLARE_COMPONENT(JetTagger)
