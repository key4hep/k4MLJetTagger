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

#include "JetTaggingAlg.h"

#include "GaudiKernel/MsgStream.h"

DECLARE_COMPONENT(JetTaggingAlg)

JetTaggingAlg::JetTaggingAlg(const std::string& aName, ISvcLocator* aSvcLoc) : Gaudi::Algorithm(aName, aSvcLoc) {
  // declare properties here like 
  declareProperty("RefinedVertexJets", m_refinedVertexJets, "Input jets collection name");
}

JetTaggingAlg::~JetTaggingAlg() {}

// initialize method - if you need sometime done once instead of per event
StatusCode JetTaggingAlg::initialize() {
  if (Gaudi::Algorithm::initialize().isFailure()) {
    return StatusCode::FAILURE;
  }
  info() << "Initialization successful" << endmsg;
  return StatusCode::SUCCESS;
}

// execute method - used to process an event
StatusCode JetTaggingAlg::execute(const EventContext&) const {
  info() << endmsg;
  info() << endmsg;
  info() << theMessage << endmsg;
  info() << endmsg;
  info() << endmsg;

  // Retrieve the RefinedVertexJets collection
  const edm4hep::ReconstructedParticle* refinedVertexJets = m_refinedVertexJets.get();
  // if (!refinedVertexJets) {
  //   error() << "Failed to retrieve RefinedVertexJets collection" << endmsg;
  //   return StatusCode::FAILURE;
  // }
  //const edm4hep::TrackerHit3DCollection* input_hits = m_input_hits.get();
  //info() << "Input Hit collection size: " << input_hits->size() << endmsg;

  //info() << "Jets? : " << refinedVertexJets->getEnergy() << endmsg;

  // Iterate over jets in the collection
  //for (size_t i = 0; i < refinedVertexJets->size(); ++i) {
  //  const auto& jet = (*refinedVertexJets)[i];
  //  info() << "Jet: "
  //         << " Energy: " << jet.getEnergy()
  //         << " Px: " << jet.getMomentum().x
  //         << " Py: " << jet.getMomentum().y
  //         << " Pz: " << jet.getMomentum().z
  //         << endmsg;
  //}

  return StatusCode::SUCCESS;
}

StatusCode JetTaggingAlg::finalize() { return Gaudi::Algorithm::finalize(); }
