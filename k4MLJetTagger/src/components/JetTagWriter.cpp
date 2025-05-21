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

#include "JetTagWriter.h"

#include <edm4hep/ParticleIDCollection.h>
#include <edm4hep/utils/ParticleIDUtils.h>

DECLARE_COMPONENT(JetTagWriter)

JetTagWriter::JetTagWriter(const std::string& name, ISvcLocator* svcLoc) : Gaudi::Algorithm(name, svcLoc) {
  declareProperty("InputJets", m_jetsHandle, "Collection of refined jets");
  declareProperty("RefinedJetTag_G", m_recoJettagGHandle, "Collection for jet flavor tag G");
  declareProperty("RefinedJetTag_U", m_recoJettagUHandle, "Collection for jet flavor tag U");
  declareProperty("RefinedJetTag_D", m_recoJettagDHandle, "Collection for jet flavor tag D");
  declareProperty("RefinedJetTag_S", m_recoJettagSHandle, "Collection for jet flavor tag S");
  declareProperty("RefinedJetTag_C", m_recoJettagCHandle, "Collection for jet flavor tag C");
  declareProperty("RefinedJetTag_B", m_recoJettagBHandle, "Collection for jet flavor tag B");
  declareProperty("RefinedJetTag_TAU", m_recoJettagTauHandle, "Collection for jet flavor tag TAU");
  declareProperty("MCJetTag", m_mcJettagHandle, "Collection for MC Jet Tag");
}

StatusCode JetTagWriter::initialize() {
  if (Gaudi::Algorithm::initialize().isFailure())
    return StatusCode::FAILURE;

  m_ths = service("THistSvc", true);
  if (!m_ths) {
    error() << "Couldn't get THistSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  m_jettag = new TTree("JetTags", "Jet flavor tags");
  if (m_ths->regTree("/rec/jetflags", m_jettag).isFailure()) {
    error() << "Couldn't register jet flags tree" << endmsg;
    return StatusCode::FAILURE;
  }

  initializeTree();

  return StatusCode::SUCCESS;
}

StatusCode JetTagWriter::execute(const EventContext&) const {
  auto evs = m_eventHeaderHandle.get();
  m_evNum = (*evs)[0].getEventNumber();
  // evNum = 0;
  info() << "Starting to write jet tags of event " << m_evNum << " into a tree..." << endmsg;

  // Get the pointers to the collections
  const edm4hep::ReconstructedParticleCollection* jet_coll_ptr = m_jetsHandle.get();
  const edm4hep::ParticleIDCollection* reco_jettag_G_coll_ptr = m_recoJettagGHandle.get();
  const edm4hep::ParticleIDCollection* reco_jettag_U_coll_ptr = m_recoJettagUHandle.get();
  const edm4hep::ParticleIDCollection* reco_jettag_D_coll_ptr = m_recoJettagDHandle.get();
  const edm4hep::ParticleIDCollection* reco_jettag_S_coll_ptr = m_recoJettagSHandle.get();
  const edm4hep::ParticleIDCollection* reco_jettag_C_coll_ptr = m_recoJettagCHandle.get();
  const edm4hep::ParticleIDCollection* reco_jettag_B_coll_ptr = m_recoJettagBHandle.get();
  const edm4hep::ParticleIDCollection* reco_jettag_TAU_coll_ptr = m_recoJettagTauHandle.get();
  const edm4hep::ParticleIDCollection* mc_jettag_coll_ptr = m_mcJettagHandle.get();
  // Create references to the collections
  const edm4hep::ReconstructedParticleCollection& jet_coll = *jet_coll_ptr;
  const edm4hep::ParticleIDCollection& reco_jettag_G_coll = *reco_jettag_G_coll_ptr;
  const edm4hep::ParticleIDCollection& reco_jettag_U_coll = *reco_jettag_U_coll_ptr;
  const edm4hep::ParticleIDCollection& reco_jettag_D_coll = *reco_jettag_D_coll_ptr;
  const edm4hep::ParticleIDCollection& reco_jettag_S_coll = *reco_jettag_S_coll_ptr;
  const edm4hep::ParticleIDCollection& reco_jettag_C_coll = *reco_jettag_C_coll_ptr;
  const edm4hep::ParticleIDCollection& reco_jettag_B_coll = *reco_jettag_B_coll_ptr;
  const edm4hep::ParticleIDCollection& reco_jettag_TAU_coll = *reco_jettag_TAU_coll_ptr;
  const edm4hep::ParticleIDCollection& mc_jettag_coll = *mc_jettag_coll_ptr;

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
    if (jetTags_G.empty() || jetTags_U.empty() || jetTags_D.empty() || jetTags_S.empty() || jetTags_C.empty() ||
        jetTags_B.empty() || jetTags_TAU.empty() || mcJetTags.empty()) {
      error() << "No PID info found for jet!" << endmsg;
      continue;
    }
    // check if the jetTags have only one value each
    if (jetTags_G.size() != 1 || jetTags_U.size() != 1 || jetTags_D.size() != 1 || jetTags_S.size() != 1 ||
        jetTags_C.size() != 1 || jetTags_B.size() != 1 || jetTags_TAU.size() != 1 || mcJetTags.size() != 1) {
      error() << "More than one PID info for one flavor found for jet!" << endmsg;
      continue;
    }

    // get the PID likelihoods
    m_scoreRecojetIsG = jetTags_G[0].getLikelihood();
    m_scoreRecoJetIsU = jetTags_U[0].getLikelihood();
    m_scoreRecoJetIsD = jetTags_D[0].getLikelihood();
    m_scoreRecoJetIsS = jetTags_S[0].getLikelihood();
    m_scoreRecoJetIsC = jetTags_C[0].getLikelihood();
    m_scoreRecoJetIsB = jetTags_B[0].getLikelihood();
    m_scoreRecoJetIsTau = jetTags_TAU[0].getLikelihood();

    // check if no dummy value is left
    if (m_scoreRecojetIsG == -9.0 || m_scoreRecoJetIsU == -9.0 || m_scoreRecoJetIsD == -9.0 ||
        m_scoreRecoJetIsS == -9.0 || m_scoreRecoJetIsC == -9.0 || m_scoreRecoJetIsB == -9.0 ||
        m_scoreRecoJetIsTau == -9.0) {
      error() << "Dummy value for probability scores still seems to be set!" << endmsg;
      continue;
    }

    // get MC jet flavor and set the corresponding bool to true
    int mc_flavor = mcJetTags[0].getPDG();
    if (mc_flavor == 21) {
      m_recojetIsG = true;
    } else if (mc_flavor == 2) {
      m_recoJetIsU = true;
    } else if (mc_flavor == 1) {
      m_recoJetIsD = true;
    } else if (mc_flavor == 3) {
      m_recoJetIsS = true;
    } else if (mc_flavor == 4) {
      m_recoJetIsC = true;
    } else if (mc_flavor == 5) {
      m_recoJetIsB = true;
    } else if (mc_flavor == 15) {
      m_recoJetIsTAU = true;
    } else {
      error() << "MC jet flavor not found!" << endmsg;
      continue;
    }

    // fill the tree
    m_jettag->Fill();
  }

  return StatusCode::SUCCESS;
}

void JetTagWriter::initializeTree() {
  m_jettag->Branch("recojet_isG", &m_recojetIsG, "recojet_isG/O");
  m_jettag->Branch("score_recojet_isG", &m_scoreRecojetIsG, "score_recojet_isG/F");
  m_jettag->Branch("m_recoJetIsU", &m_recoJetIsU, "m_recoJetIsU/O");
  m_jettag->Branch("m_scoreRecoJetIsU", &m_scoreRecoJetIsU, "m_scoreRecoJetIsU/F");
  m_jettag->Branch("m_recoJetIsD", &m_recoJetIsD, "m_recoJetIsD/O");
  m_jettag->Branch("m_scoreRecoJetIsD", &m_scoreRecoJetIsD, "m_scoreRecoJetIsD/F");
  m_jettag->Branch("m_recoJetIsS", &m_recoJetIsS, "m_recoJetIsS/O");
  m_jettag->Branch("m_scoreRecoJetIsS", &m_scoreRecoJetIsS, "m_scoreRecoJetIsS/F");
  m_jettag->Branch("m_recoJetIsC", &m_recoJetIsC, "m_recoJetIsC/O");
  m_jettag->Branch("m_scoreRecoJetIsC", &m_scoreRecoJetIsC, "m_scoreRecoJetIsC/F");
  m_jettag->Branch("m_recoJetIsB", &m_recoJetIsB, "m_recoJetIsB/O");
  m_jettag->Branch("m_scoreRecoJetIsB", &m_scoreRecoJetIsB, "m_scoreRecoJetIsB/F");
  m_jettag->Branch("m_recoJetIsTAU", &m_recoJetIsTAU, "m_recoJetIsTAU/O");
  m_jettag->Branch("m_scoreRecoJetIsTAU", &m_scoreRecoJetIsTau, "m_scoreRecoJetIsTAU/F");

  return;
}

void JetTagWriter::cleanTree() const {
  m_recojetIsG = false;
  m_recoJetIsU = false;
  m_recoJetIsD = false;
  m_recoJetIsS = false;
  m_recoJetIsC = false;
  m_recoJetIsB = false;
  m_recoJetIsTAU = false;

  float dummy_score = -9.0;
  m_scoreRecoJetIsTau = dummy_score;
  m_scoreRecojetIsG = dummy_score;
  m_scoreRecoJetIsU = dummy_score;
  m_scoreRecoJetIsD = dummy_score;
  m_scoreRecoJetIsS = dummy_score;
  m_scoreRecoJetIsC = dummy_score;
  m_scoreRecoJetIsB = dummy_score;

  return;
}

StatusCode JetTagWriter::finalize() {
  if (Gaudi::Algorithm::finalize().isFailure())
    return StatusCode::FAILURE;

  return StatusCode::SUCCESS;
}
