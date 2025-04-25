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
#include <edm4hep/utils/ParticleIDUtils.h>
#include <podio/Frame.h>


#include <nlohmann/json.hpp> // Include a JSON parsing library
#include <fstream>

#include "Structs.h"
#include "JetObservablesRetriever.h"
#include "JetTagWriter.h"
#include "Helpers.h"

DECLARE_COMPONENT(JetTagWriter)

JetTagWriter::JetTagWriter(const std::string& name, ISvcLocator* svcLoc) : Gaudi::Algorithm(name, svcLoc) {
    declareProperty("InputJets", jets_handle, "Collection of refined jets");
    declareProperty("RefinedJetTag_G", reco_jettag_G_handle, "Collection for jet flavor tag G");
    declareProperty("RefinedJetTag_U", reco_jettag_U_handle, "Collection for jet flavor tag U");
    declareProperty("RefinedJetTag_D", reco_jettag_D_handle, "Collection for jet flavor tag D");
    declareProperty("RefinedJetTag_S", reco_jettag_S_handle, "Collection for jet flavor tag S");
    declareProperty("RefinedJetTag_C", reco_jettag_C_handle, "Collection for jet flavor tag C");
    declareProperty("RefinedJetTag_B", reco_jettag_B_handle, "Collection for jet flavor tag B");
    declareProperty("RefinedJetTag_TAU", reco_jettag_TAU_handle, "Collection for jet flavor tag TAU");
    declareProperty("MCJetTag", mc_jettag_handle, "Collection for MC Jet Tag");
}

StatusCode JetTagWriter::initialize() {
  if (Gaudi::Algorithm::initialize().isFailure()) return StatusCode::FAILURE;

  m_ths = service("THistSvc", true);
  if (!m_ths) {
    error() << "Couldn't get THistSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  t_jettag = new TTree ("JetTags", "Jet flavor tags");
  if (m_ths->regTree("/rec/jetflags", t_jettag).isFailure()) {
    error() << "Couldn't register jet flags tree" << endmsg;
    return StatusCode::FAILURE;
  }

  initializeTree();

  return StatusCode::SUCCESS;
}

StatusCode JetTagWriter::execute(const EventContext&) const {

  auto evs = ev_handle.get();
  evNum = (*evs)[0].getEventNumber();
  //evNum = 0;
  info() << "Starting to write jet tags of event " << evNum << " into a tree..." << endmsg;

  // Get the pointers to the collections
  const edm4hep::ReconstructedParticleCollection *jet_coll_ptr = jets_handle.get();
  const edm4hep::ParticleIDCollection *reco_jettag_G_coll_ptr = reco_jettag_G_handle.get();
  const edm4hep::ParticleIDCollection *reco_jettag_U_coll_ptr = reco_jettag_U_handle.get();
  const edm4hep::ParticleIDCollection *reco_jettag_D_coll_ptr = reco_jettag_D_handle.get();
  const edm4hep::ParticleIDCollection *reco_jettag_S_coll_ptr = reco_jettag_S_handle.get();
  const edm4hep::ParticleIDCollection *reco_jettag_C_coll_ptr = reco_jettag_C_handle.get();
  const edm4hep::ParticleIDCollection *reco_jettag_B_coll_ptr = reco_jettag_B_handle.get();
  const edm4hep::ParticleIDCollection *reco_jettag_TAU_coll_ptr = reco_jettag_TAU_handle.get();
  const edm4hep::ParticleIDCollection *mc_jettag_coll_ptr = mc_jettag_handle.get();
  // Create references to the collections
  const edm4hep::ReconstructedParticleCollection &jet_coll = *jet_coll_ptr;
  const edm4hep::ParticleIDCollection &reco_jettag_G_coll = *reco_jettag_G_coll_ptr;
  const edm4hep::ParticleIDCollection &reco_jettag_U_coll = *reco_jettag_U_coll_ptr;
  const edm4hep::ParticleIDCollection &reco_jettag_D_coll = *reco_jettag_D_coll_ptr;
  const edm4hep::ParticleIDCollection &reco_jettag_S_coll = *reco_jettag_S_coll_ptr;
  const edm4hep::ParticleIDCollection &reco_jettag_C_coll = *reco_jettag_C_coll_ptr;
  const edm4hep::ParticleIDCollection &reco_jettag_B_coll = *reco_jettag_B_coll_ptr;
  const edm4hep::ParticleIDCollection &reco_jettag_TAU_coll = *reco_jettag_TAU_coll_ptr;
  const edm4hep::ParticleIDCollection &mc_jettag_coll = *mc_jettag_coll_ptr;

  auto jetTag_G_Handler = edm4hep::utils::PIDHandler::from(reco_jettag_G_coll);
  auto jetTag_U_Handler = edm4hep::utils::PIDHandler::from(reco_jettag_U_coll);
  auto jetTag_D_Handler = edm4hep::utils::PIDHandler::from(reco_jettag_D_coll);
  auto jetTag_S_Handler = edm4hep::utils::PIDHandler::from(reco_jettag_S_coll);
  auto jetTag_C_Handler = edm4hep::utils::PIDHandler::from(reco_jettag_C_coll);
  auto jetTag_B_Handler = edm4hep::utils::PIDHandler::from(reco_jettag_B_coll);
  auto jetTag_TAU_Handler = edm4hep::utils::PIDHandler::from(reco_jettag_TAU_coll);
  auto mcJetTag_Handler = edm4hep::utils::PIDHandler::from(mc_jettag_coll);

  // loop over all jets and get the PID likelihoods
  for (const auto jet : jet_coll) {

    cleanTree(); // set all values to -9.0

    auto jetTags_G = jetTag_G_Handler.getPIDs(jet);
    auto jetTags_U = jetTag_U_Handler.getPIDs(jet);
    auto jetTags_D = jetTag_D_Handler.getPIDs(jet);
    auto jetTags_S = jetTag_S_Handler.getPIDs(jet);
    auto jetTags_C = jetTag_C_Handler.getPIDs(jet);
    auto jetTags_B = jetTag_B_Handler.getPIDs(jet);
    auto jetTags_TAU = jetTag_TAU_Handler.getPIDs(jet);
    auto mcJetTags = mcJetTag_Handler.getPIDs(jet);

    // check if the PID info is available
    if (jetTags_G.empty() || jetTags_U.empty() || jetTags_D.empty() || jetTags_S.empty() || jetTags_C.empty() || jetTags_B.empty() || jetTags_TAU.empty() || mcJetTags.empty()) {
      error() << "No PID info found for jet!" << endmsg;
      continue;
    }
    // check if the jetTags have only one value each
    if (jetTags_G.size() != 1 || jetTags_U.size() != 1 || jetTags_D.size() != 1 || jetTags_S.size() != 1 || jetTags_C.size() != 1 || jetTags_B.size() != 1 || jetTags_TAU.size() != 1 || mcJetTags.size() != 1) {
      error() << "More than one PID info for one flavor found for jet!" << endmsg;
      continue;
    }

    // get the PID likelihoods
    score_recojet_isG = jetTags_G[0].getLikelihood();
    score_recojet_isU = jetTags_U[0].getLikelihood();
    score_recojet_isD = jetTags_D[0].getLikelihood();
    score_recojet_isS = jetTags_S[0].getLikelihood();
    score_recojet_isC = jetTags_C[0].getLikelihood();
    score_recojet_isB = jetTags_B[0].getLikelihood();
    score_recojet_isTAU = jetTags_TAU[0].getLikelihood();

    // check if no dummy value is left
    if (score_recojet_isG == -9.0 || score_recojet_isU == -9.0 || score_recojet_isD == -9.0 || score_recojet_isS == -9.0 || score_recojet_isC == -9.0 || score_recojet_isB == -9.0 || score_recojet_isTAU == -9.0) {
      error() << "Dummy value for probability scores still seems to be set!" << endmsg;
      continue;
    }

    // get MC jet flavor and set the corresponding bool to true
    int mc_flavor = mcJetTags[0].getPDG();
    if (mc_flavor == 21) {
      recojet_isG = true;
    } else if (mc_flavor == 2) {
      recojet_isU = true;
    } else if (mc_flavor == 1) {
      recojet_isD = true;
    } else if (mc_flavor == 3) {
      recojet_isS = true;
    } else if (mc_flavor == 4) {
      recojet_isC = true;
    } else if (mc_flavor == 5) {
      recojet_isB = true;
    } else if (mc_flavor == 15) {
      recojet_isTAU = true;
    } else {
      error() << "MC jet flavor not found!" << endmsg;
      continue;
    }

    // fill the tree
    t_jettag -> Fill();

  }


  return StatusCode::SUCCESS;
}

void JetTagWriter::initializeTree() {

  t_jettag->Branch("recojet_isG", &recojet_isG, "recojet_isG/O");
  t_jettag->Branch("score_recojet_isG", &score_recojet_isG, "score_recojet_isG/F");
  t_jettag->Branch("recojet_isU", &recojet_isU, "recojet_isU/O");
  t_jettag->Branch("score_recojet_isU", &score_recojet_isU, "score_recojet_isU/F");
  t_jettag->Branch("recojet_isD", &recojet_isD, "recojet_isD/O");
  t_jettag->Branch("score_recojet_isD", &score_recojet_isD, "score_recojet_isD/F");
  t_jettag->Branch("recojet_isS", &recojet_isS, "recojet_isS/O");
  t_jettag->Branch("score_recojet_isS", &score_recojet_isS, "score_recojet_isS/F");
  t_jettag->Branch("recojet_isC", &recojet_isC, "recojet_isC/O");
  t_jettag->Branch("score_recojet_isC", &score_recojet_isC, "score_recojet_isC/F");
  t_jettag->Branch("recojet_isB", &recojet_isB, "recojet_isB/O");
  t_jettag->Branch("score_recojet_isB", &score_recojet_isB, "score_recojet_isB/F");
  t_jettag->Branch("recojet_isTAU", &recojet_isTAU, "recojet_isTAU/O");
  t_jettag->Branch("score_recojet_isTAU", &score_recojet_isTAU, "score_recojet_isTAU/F");

  return;
}

void JetTagWriter::cleanTree() const {

  recojet_isG = false;
  recojet_isU = false;
  recojet_isD = false;
  recojet_isS = false;
  recojet_isC = false;
  recojet_isB = false;
  recojet_isTAU = false;

  float dummy_score = -9.0;
  score_recojet_isTAU = dummy_score;
  score_recojet_isG = dummy_score;
  score_recojet_isU = dummy_score;
  score_recojet_isD = dummy_score;
  score_recojet_isS = dummy_score;
  score_recojet_isC = dummy_score;
  score_recojet_isB = dummy_score;

  return;
}

StatusCode JetTagWriter::finalize() {
  if (Gaudi::Algorithm::finalize().isFailure()) return StatusCode::FAILURE;

  return StatusCode::SUCCESS;
}