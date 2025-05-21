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

#include "Gaudi/Property.h"
#include "GaudiKernel/MsgStream.h"
#include "k4FWCore/Transformer.h"
#include "k4Interface/IGeoSvc.h" // for Bfield
#include <edm4hep/ParticleIDCollection.h>
#include <edm4hep/ReconstructedParticleCollection.h>
#include <edm4hep/VertexCollection.h>

#include <nlohmann/json.hpp> // Include a JSON parsing library

#include "Helpers.h"
#include "JetObservablesRetriever.h"
#include "Structs.h"
#include "WeaverInterface.h"

/**
 * @class JetTagger
 * @brief Gaudi transformer that builds edm4hep::ParticleIDCollection objects (!plural, one for each flavor) for each
 * jet in edm4hep::ReconstructedParticleCollection.
 *
 * We retrieve a description of the jet constituents which serve as an input to a neural network. The network is loaded
 * as an ONNX model. The inference is run on each jet. The output of the network is a vector of probabilities for each
 * jet flavor. We create one ParticleID collection per flavor create, link it to the jet and set the likelihood and PDG
 * number.
 *
 * @author Sara Aumiller
 */
struct JetTagger : k4FWCore::Transformer<std::vector<edm4hep::ParticleIDCollection>(
                       const edm4hep::ReconstructedParticleCollection&, const edm4hep::VertexCollection&)> {
  JetTagger(const std::string& name, ISvcLocator* svcLoc)
      : Transformer(
            name, svcLoc,
            {KeyValues("InputJets", {"RefinedVertexJets"}), KeyValues("InputPrimaryVertices", {"PrimaryVertices"})},
            {KeyValues("OutputIDCollections", {"RefinedJetTags"})}) {}

  // operator
  std::vector<edm4hep::ParticleIDCollection> operator()(const edm4hep::ReconstructedParticleCollection& inputJets,
                                                        const edm4hep::VertexCollection& primVerticies) const override {
    info() << "Tagging " << inputJets.size() << " input jets" << endmsg;

    // create n ParticleIDCollection objects, one for each flavor & retrieve the PDG number for each flavor
    std::vector<edm4hep::ParticleIDCollection> tagCollections;
    tagCollections.resize(m_flavorNames.size());

    for (const auto& jet : inputJets) {
      // retrieve the input observables to the network from the jet
      Jet j = m_retriever->retrieve_input_observables(jet, primVerticies);

      // Convert the Jet object to the input format for the ONNX model
      const auto jet_const_data = from_Jet_to_onnx_input(j, m_vars);

      // Run inference on the input variables - returns the 7 probabilities for each jet flavor
      const auto probabilities = m_weaver->run(jet_const_data);

      // For debugging: Compute the highest probability & its flavor
      auto maxIt = std::max_element(probabilities.begin(), probabilities.end());
      float maxProb = *maxIt;
      auto maxIndex = std::distance(probabilities.begin(), maxIt);
      debug() << "Jet has highest probability for flavor " << m_flavorNames[maxIndex] << " with " << maxProb << endmsg;

      if (probabilities.size() != m_flavorNames.size()) {
        error() << "Number of probabilities returned by the network does not match number of flavors stated in the "
                   "network config json"
                << endmsg;
      }

      // fill the ParticleIDCollection objects
      for (unsigned int i = 0; i < m_flavorNames.size(); i++) {
        auto jetTag = tagCollections[i].create();
        jetTag.setParticle(jet);
        jetTag.setLikelihood(probabilities[i]);
        jetTag.setPDG(m_pdgFlavors[i]);
      }
    }

    // For debugging: print if the ParticleIDCollection objects are filled correctly
    for (unsigned int i = 0; i < tagCollections.size(); i++) {
      debug() << "ParticleID collection for " << m_flavorNames[i] << " has size: " << tagCollections[i].size()
              << " with likelihoods " << tagCollections[i].likelihood() << " and PDGs " << tagCollections[i].PDG()
              << endmsg;
    }

    return tagCollections;
  };

  // initialize
  StatusCode initialize() override {
    // Load the JSON configuration file and retrieve the flavor names
    auto json_config = loadJsonFile(m_jsonPath);
    m_flavorNames =
        json_config["output_names"]; // e.g. "recojet_isX" with X being the jet flavor (G, U, S, C, B, D, TAU)

    // check if flavorNames matches order and size of the output collections
    if (!check_flavors(m_flavorNames, m_flavorCollectionNames)) {
      error() << "ATTENTION! Output flavor collection names MUST match ONNX model output flavors!" << endmsg;
      info() << "Flavors expected from network in this order: " << m_flavorNames << endmsg;
    }
    for (const auto& flavor : m_flavorNames) {
      m_pdgFlavors.push_back(to_PDGflavor.at(flavor)); // retrieve the PDG number from the flavor name
    }

    // WeaverInterface object

    // retrieve the input variable to onnx model from json file
    for (const auto& var : json_config["pf_features"]["var_names"]) {
      m_vars.push_back(var.get<std::string>());
    }
    for (const auto& var : json_config["pf_vectors"]["var_names"]) { // not sure if this is the solution here
      m_vars.push_back(var.get<std::string>());
    }
    // variables in pf_points are already included in pf_features

    // Create the WeaverInterface object
    m_weaver = std::make_unique<WeaverInterface>(m_modelPath, m_jsonPath, m_vars);

    // JetObservablesRetriever object
    m_retriever = std::make_unique<JetObservablesRetriever>();
    // get B field from detector (this is computatially expensive, so we hardcode it for now)
    // dd4hep::Detector* theDetector = Gaudi::svcLocator()->service<IGeoSvc>("GeoSvc")->getDetector();
    // double Bfield = getBzAtOrigin(theDetector);

    m_retriever->Bz = 2.0; // hardcoded for now

    return StatusCode::SUCCESS;
  }

  // properties
private:
  std::vector<std::string> m_flavorNames; // e.g. "recojet_isX" with X being the jet flavor (G, U, S, C, B, D, TAU)
  std::vector<int> m_pdgFlavors;
  rv::RVec<std::string> m_vars; // e.g. pfcand_isEl, ... input names that onnx model expects

  mutable std::unique_ptr<WeaverInterface> m_weaver;
  mutable std::unique_ptr<JetObservablesRetriever> m_retriever;

  Gaudi::Property<std::string> m_modelPath{
      this, "model_path", "/eos/experiment/fcc/ee/jet_flavour_tagging/fullsim_test_spring2024/fullsimCLD240_2mio.onnx",
      "Path to the ONNX model"};
  Gaudi::Property<std::string> m_jsonPath{
      this, "json_path",
      "/eos/experiment/fcc/ee/jet_flavour_tagging/fullsim_test_spring2024/preprocess_fullsimCLD240_2mio.json",
      "Path to the JSON configuration file for the ONNX model"};
  Gaudi::Property<std::vector<std::string>> m_flavorCollectionNames{
      this,
      "flavor_collection_names",
      {"RefinedJetTag_G", "RefinedJetTag_U", "RefinedJetTag_S", "RefinedJetTag_C", "RefinedJetTag_B", "RefinedJetTag_D",
       "RefinedJetTag_TAU"},
      "Names of the output collections. Order, size and flavor labels _X must match the network configuration."};
};

DECLARE_COMPONENT(JetTagger)
