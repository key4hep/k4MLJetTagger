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
#include "JetPIDRetriever.h"
#include "Helpers.h"

DECLARE_COMPONENT(JetPIDRetriever)

JetPIDRetriever::JetPIDRetriever(const std::string& name, ISvcLocator* svcLoc) : Gaudi::Algorithm(name, svcLoc) {
    declareProperty("InputJets", jets_handle, "Collection of refined jets");
    declareProperty("RefinedJetTag_G", reco_jettag_G_handle, "Collection for jet flavor tag G");
    declareProperty("Pid", pid_handle, "Dummy Collection for ParticleID");
    // declareProperty("RefinedJetTag_U", reco_jettag_U_handle, "Collection for jet flavor tag U");
    // declareProperty("RefinedJetTag_D", reco_jettag_D_handle, "Collection for jet flavor tag D");
    // declareProperty("RefinedJetTag_S", reco_jettag_S_handle, "Collection for jet flavor tag S");
    // declareProperty("RefinedJetTag_C", reco_jettag_C_handle, "Collection for jet flavor tag C");
    // declareProperty("RefinedJetTag_B", reco_jettag_B_handle, "Collection for jet flavor tag B");
    // declareProperty("RefinedJetTag_TAU", reco_jettag_TAU_handle, "Collection for jet flavor tag TAU");
    // declareProperty("MCJetTag", mc_jettag_handle, "Collection for MC Jet Tag");
}

StatusCode JetPIDRetriever::initialize() {
  if (Gaudi::Algorithm::initialize().isFailure()) return StatusCode::FAILURE;

  m_ths = service("THistSvc", true);
  if (!m_ths) {
    error() << "Couldn't get THistSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  t_jettag = new TTree ("JetTags", "Jet flavor tags");
  if (m_ths->regTree("/rec/jetconst", t_jettag).isFailure()) {
    error() << "Couldn't register jet constituent tree" << endmsg;
    return StatusCode::FAILURE;
  }

  initializeTree();

  return StatusCode::SUCCESS;
}

StatusCode JetPIDRetriever::execute(const EventContext&) const {

  auto evs = ev_handle.get();
  evNum = (*evs)[0].getEventNumber();
  //evNum = 0;
  info() << "Event number = " << evNum << endmsg;

  // Get the pointers to the collections
  const edm4hep::ReconstructedParticleCollection *jet_coll_ptr = jets_handle.get();
  const edm4hep::ParticleIDCollection *reco_jettag_G_coll_ptr = reco_jettag_G_handle.get();
  const edm4hep::ParticleIDCollection *pid_coll_ptr = pid_handle.get();
  // const edm4hep::ParticleIDCollection *reco_jettag_U_coll_ptr = reco_jettag_U_handle.get();
  // const edm4hep::ParticleIDCollection *reco_jettag_D_coll_ptr = reco_jettag_D_handle.get();
  // const edm4hep::ParticleIDCollection *reco_jettag_S_coll_ptr = reco_jettag_S_handle.get();
  // const edm4hep::ParticleIDCollection *reco_jettag_C_coll_ptr = reco_jettag_C_handle.get();
  // const edm4hep::ParticleIDCollection *reco_jettag_B_coll_ptr = reco_jettag_B_handle.get();
  // const edm4hep::ParticleIDCollection *reco_jettag_TAU_coll_ptr = reco_jettag_TAU_handle.get();
  // const edm4hep::ParticleIDCollection *mc_jettag_coll_ptr = mc_jettag_handle.get();
  // Create references to the collections
  const edm4hep::ReconstructedParticleCollection &jet_coll = *jet_coll_ptr;
  const edm4hep::ParticleIDCollection &reco_jettag_G_coll = *reco_jettag_G_coll_ptr;
  const edm4hep::ParticleIDCollection &pid_coll = *pid_coll_ptr;
  const auto &events = *evs;
  // const edm4hep::ParticleIDCollection &reco_jettag_U_coll = *reco_jettag_U_coll_ptr;
  // const edm4hep::ParticleIDCollection &reco_jettag_D_coll = *reco_jettag_D_coll_ptr;
  // const edm4hep::ParticleIDCollection &reco_jettag_S_coll = *reco_jettag_S_coll_ptr;
  // const edm4hep::ParticleIDCollection &reco_jettag_C_coll = *reco_jettag_C_coll_ptr;
  // const edm4hep::ParticleIDCollection &reco_jettag_B_coll = *reco_jettag_B_coll_ptr;
  // const edm4hep::ParticleIDCollection &reco_jettag_TAU_coll = *reco_jettag_TAU_coll_ptr;
  // const edm4hep::ParticleIDCollection &mc_jettag_coll = *mc_jettag_coll_ptr;

  auto jetTag_G_Handler = edm4hep::utils::PIDHandler::from(pid_coll);
  // for (const auto jet : jet_coll) {
  //   auto jetTags = jetTag_G_Handler.getPIDs(jet);
  //   if (!jetTags.empty()) {
  //     auto prob = jetTags[0].getLikelihood();
  //     info() << "G has likelihood " << prob << endmsg;
  //   }
  // }
  // t_jettag -> Fill();

  info () << "doing nothing works" << endmsg;

  return StatusCode::SUCCESS;
}

void JetPIDRetriever::initializeTree() {

  t_jettag = new TTree("JetTags", "Jet flavor tags");

  recojet_isG = new std::vector<float>();
  score_recojet_isG = new std::vector<float>();
  recojet_isU = new std::vector<float>();
  score_recojet_isU = new std::vector<float>();
  recojet_isD = new std::vector<float>();
  score_recojet_isD = new std::vector<float>();
  recojet_isS = new std::vector<float>();
  score_recojet_isS = new std::vector<float>();
  recojet_isC = new std::vector<float>();
  score_recojet_isC = new std::vector<float>();
  recojet_isB = new std::vector<float>();
  score_recojet_isB = new std::vector<float>();
  recojet_isTAU = new std::vector<float>();
  score_recojet_isTAU = new std::vector<float>();

  t_jettag->Branch("recojet_isG", &recojet_isG);
  t_jettag->Branch("score_recojet_isG", &score_recojet_isG);
  t_jettag->Branch("recojet_isU", &recojet_isU);
  t_jettag->Branch("score_recojet_isU", &score_recojet_isU);
  t_jettag->Branch("recojet_isD", &recojet_isD);
  t_jettag->Branch("score_recojet_isD", &score_recojet_isD);
  t_jettag->Branch("recojet_isS", &recojet_isS);
  t_jettag->Branch("score_recojet_isS", &score_recojet_isS);
  t_jettag->Branch("recojet_isC", &recojet_isC);
  t_jettag->Branch("score_recojet_isC", &score_recojet_isC);
  t_jettag->Branch("recojet_isB", &recojet_isB);
  t_jettag->Branch("score_recojet_isB", &score_recojet_isB);
  t_jettag->Branch("recojet_isTAU", &recojet_isTAU);
  t_jettag->Branch("score_recojet_isTAU", &score_recojet_isTAU);

  return;
}

void JetPIDRetriever::cleanTree() const {
  recojet_isG->clear();
  score_recojet_isG->clear();
  recojet_isU->clear();
  score_recojet_isU->clear();
  recojet_isD->clear();
  score_recojet_isD->clear();
  recojet_isS->clear();
  score_recojet_isS->clear();
  recojet_isC->clear();
  score_recojet_isC->clear();
  recojet_isB->clear();
  score_recojet_isB->clear();
  recojet_isTAU->clear();
  score_recojet_isTAU->clear();

  return;
}

StatusCode JetPIDRetriever::finalize() {
  if (Gaudi::Algorithm::finalize().isFailure()) return StatusCode::FAILURE;

  return StatusCode::SUCCESS;
}