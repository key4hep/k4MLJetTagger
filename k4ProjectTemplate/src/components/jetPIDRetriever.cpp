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


#include <nlohmann/json.hpp> // Include a JSON parsing library
#include <fstream>

#include "Structs.h"
#include "JetObservablesRetriever.h"
#include "JetPIDRetriever.h"
#include "Helpers.h"

DECLARE_COMPONENT(JetPIDRetriever)

JetPIDRetriever::JetPIDRetriever(const std::string& name, ISvcLocator* svcLoc) : Gaudi::Algorithm(name, svcLoc) {
    declareProperty("RefindJetTag_isG", reco_jettag_isG_handle, "Collection for jet flavor tag G");
    declareProperty("RefindJetTag_isU", reco_jettag_isU_handle, "Collection for jet flavor tag U");
    declareProperty("RefindJetTag_isD", reco_jettag_isD_handle, "Collection for jet flavor tag D");
    declareProperty("RefindJetTag_isS", reco_jettag_isS_handle, "Collection for jet flavor tag S");
    declareProperty("RefindJetTag_isC", reco_jettag_isC_handle, "Collection for jet flavor tag C");
    declareProperty("RefindJetTag_isB", reco_jettag_isB_handle, "Collection for jet flavor tag B");
    declareProperty("RefindJetTag_isTAU", reco_jettag_isTAU_handle, "Collection for jet flavor tag TAU");
    declareProperty("MCJetTag", mc_jettag_handle, "Collection for MC Jet Tag");
}

StatusCode JetPIDRetriever::initialize() {
  if (Gaudi::Algorithm::initialize().isFailure()) return StatusCode::FAILURE;

  m_ths = service("THistSvc", true);
  if (!m_ths) {
    error() << "Couldn't get THistSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  t_jettag = new TTree ("JetTags", "Jet flavor tags");
  if (m_ths->regTree("/rec/jetconst", t_jetcst).isFailure()) {
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
  const edm4hep::ReconstructedParticleCollection *jet_coll_ptr = reco_jet_handle.get();
  const edm4hep::VertexCollection *prim_vertex_coll_ptr = reco_vertex_handle.get();
  const edm4hep::ParticleIDCollection *reco_jettag_isG_coll_ptr = reco_jettag_isG_handle.get();
  const edm4hep::ParticleIDCollection *reco_jettag_isU_coll_ptr = reco_jettag_isU_handle.get();
  const edm4hep::ParticleIDCollection *reco_jettag_isD_coll_ptr = reco_jettag_isD_handle.get();
  const edm4hep::ParticleIDCollection *reco_jettag_isS_coll_ptr = reco_jettag_isS_handle.get();
  const edm4hep::ParticleIDCollection *reco_jettag_isC_coll_ptr = reco_jettag_isC_handle.get();
  const edm4hep::ParticleIDCollection *reco_jettag_isB_coll_ptr = reco_jettag_isB_handle.get();
  const edm4hep::ParticleIDCollection *reco_jettag_isTAU_coll_ptr = reco_jettag_isTAU_handle.get();
  const edm4hep::ParticleIDCollection *mc_jettag_coll_ptr = mc_jettag_handle.get();
  // Create references to the collections
  const edm4hep::ReconstructedParticleCollection &jet_coll = *jet_coll_ptr;
  const edm4hep::VertexCollection &prim_vertex_coll = *prim_vertex_coll_ptr;
  const edm4hep::ParticleIDCollection &reco_jettag_isG_coll = *reco_jettag_isG_coll_ptr;
  const edm4hep::ParticleIDCollection &reco_jettag_isU_coll = *reco_jettag_isU_coll_ptr;
  const edm4hep::ParticleIDCollection &reco_jettag_isD_coll = *reco_jettag_isD_coll_ptr;
  const edm4hep::ParticleIDCollection &reco_jettag_isS_coll = *reco_jettag_isS_coll_ptr;
  const edm4hep::ParticleIDCollection &reco_jettag_isC_coll = *reco_jettag_isC_coll_ptr;
  const edm4hep::ParticleIDCollection &reco_jettag_isB_coll = *reco_jettag_isB_coll_ptr;
  const edm4hep::ParticleIDCollection &reco_jettag_isTAU_coll = *reco_jettag_isTAU_coll_ptr;
  const edm4hep::ParticleIDCollection &mc_jettag_coll = *mc_jettag_coll_ptr;

  auto jetTagHandler = edm4hep::utils::PIDHandler::from(jetTags); // where the jetTags is the ParticleIDCollection
  for (const auto jet : jetCollection) {
    auto jetTags = jetTagHandler.getPIDs(jet);
    if (!jetTags.empty()) {
      auto jetTag = jetTags[0].getType();
    }
  }
  t_jettag -> Fill();

  return StatusCode::SUCCESS;
}

void JetPIDRetriever::initializeTree() {

  pfcand_erel_log = new std::vector<float>();
  pfcand_thetarel = new std::vector<float>();
  pfcand_phirel = new std::vector<float>();
  pfcand_e = new std::vector<float>();
  pfcand_p = new std::vector<float>();
  pfcand_type = new std::vector<int>();
  pfcand_charge = new std::vector<int>();
  pfcand_isEl = new std::vector<int>();
  pfcand_isMu = new std::vector<int>();
  pfcand_isGamma = new std::vector<int>();
  pfcand_isChargedHad = new std::vector<int>();
  pfcand_isNeutralHad = new std::vector<int>();
  pfcand_dndx = new std::vector<float>();
  pfcand_tof = new std::vector<float>();
  pfcand_cov_omegaomega = new std::vector<float>();
  pfcand_cov_tanLambdatanLambda = new std::vector<float>();
  pfcand_cov_phiphi = new std::vector<float>();
  pfcand_cov_d0d0 = new std::vector<float>();
  pfcand_cov_z0z0 = new std::vector<float>();
  pfcand_cov_d0z0 = new std::vector<float>();
  pfcand_cov_phid0 = new std::vector<float>();
  pfcand_cov_tanLambdaz0 = new std::vector<float>();
  pfcand_cov_d0omega = new std::vector<float>();
  pfcand_cov_d0tanLambda = new std::vector<float>();
  pfcand_cov_phiomega = new std::vector<float>();
  pfcand_cov_phiz0 = new std::vector<float>();
  pfcand_cov_phitanLambda = new std::vector<float>();
  pfcand_cov_omegaz0 = new std::vector<float>();
  pfcand_cov_omegatanLambda = new std::vector<float>();
  pfcand_d0 = new std::vector<float>();
  pfcand_z0 = new std::vector<float>();
  pfcand_Sip2dVal = new std::vector<float>();
  pfcand_Sip2dSig = new std::vector<float>();
  pfcand_Sip3dVal = new std::vector<float>();
  pfcand_Sip3dSig = new std::vector<float>();
  pfcand_JetDistVal = new std::vector<float>();
  pfcand_JetDistSig = new std::vector<float>();

  t_jetcst->Branch("pfcand_erel_log", &pfcand_erel_log);
  t_jetcst->Branch("pfcand_thetarel", &pfcand_thetarel);
  t_jetcst->Branch("pfcand_phirel", &pfcand_phirel);
  t_jetcst->Branch("pfcand_e", &pfcand_e);
  t_jetcst->Branch("pfcand_p", &pfcand_p);
  t_jetcst->Branch("pfcand_type", &pfcand_type);
  t_jetcst->Branch("pfcand_charge", &pfcand_charge);
  t_jetcst->Branch("pfcand_isEl", &pfcand_isEl);
  t_jetcst->Branch("pfcand_isMu", &pfcand_isMu);
  t_jetcst->Branch("pfcand_isGamma", &pfcand_isGamma);
  t_jetcst->Branch("pfcand_isChargedHad", &pfcand_isChargedHad);
  t_jetcst->Branch("pfcand_isNeutralHad", &pfcand_isNeutralHad);
  t_jetcst->Branch("pfcand_dndx", &pfcand_dndx);
  t_jetcst->Branch("pfcand_tof", &pfcand_tof);
  t_jetcst->Branch("pfcand_cov_omegaomega", &pfcand_cov_omegaomega);
  t_jetcst->Branch("pfcand_cov_tanLambdatanLambda", &pfcand_cov_tanLambdatanLambda);
  t_jetcst->Branch("pfcand_cov_phiphi", &pfcand_cov_phiphi);
  t_jetcst->Branch("pfcand_cov_d0d0", &pfcand_cov_d0d0);
  t_jetcst->Branch("pfcand_cov_z0z0", &pfcand_cov_z0z0);
  t_jetcst->Branch("pfcand_cov_d0z0", &pfcand_cov_d0z0);
  t_jetcst->Branch("pfcand_cov_phid0", &pfcand_cov_phid0);
  t_jetcst->Branch("pfcand_cov_tanLambdaz0", &pfcand_cov_tanLambdaz0);
  t_jetcst->Branch("pfcand_cov_d0omega", &pfcand_cov_d0omega);
  t_jetcst->Branch("pfcand_cov_d0tanLambda", &pfcand_cov_d0tanLambda);
  t_jetcst->Branch("pfcand_cov_phiomega", &pfcand_cov_phiomega);
  t_jetcst->Branch("pfcand_cov_phiz0", &pfcand_cov_phiz0);
  t_jetcst->Branch("pfcand_cov_phitanLambda", &pfcand_cov_phitanLambda);
  t_jetcst->Branch("pfcand_cov_omegaz0", &pfcand_cov_omegaz0);
  t_jetcst->Branch("pfcand_cov_omegatanLambda", &pfcand_cov_omegatanLambda);
  t_jetcst->Branch("pfcand_d0", &pfcand_d0);
  t_jetcst->Branch("pfcand_z0", &pfcand_z0);
  t_jetcst->Branch("pfcand_Sip2dVal", &pfcand_Sip2dVal);
  t_jetcst->Branch("pfcand_Sip2dSig", &pfcand_Sip2dSig);
  t_jetcst->Branch("pfcand_Sip3dVal", &pfcand_Sip3dVal);
  t_jetcst->Branch("pfcand_Sip3dSig", &pfcand_Sip3dSig);
  t_jetcst->Branch("pfcand_JetDistVal", &pfcand_JetDistVal);
  t_jetcst->Branch("pfcand_JetDistSig", &pfcand_JetDistSig);

  return;
}

void JetPIDRetriever::cleanTree() const {
  pfcand_erel_log->clear();
  pfcand_thetarel->clear();
  pfcand_phirel->clear();
  pfcand_e->clear();
  pfcand_p->clear();
  pfcand_type->clear();
  pfcand_charge->clear();
  pfcand_isEl->clear();
  pfcand_isMu->clear();
  pfcand_isGamma->clear();
  pfcand_isChargedHad->clear();
  pfcand_isNeutralHad->clear();
  pfcand_dndx->clear();
  pfcand_tof->clear();
  pfcand_cov_omegaomega->clear();
  pfcand_cov_tanLambdatanLambda->clear();
  pfcand_cov_phiphi->clear();
  pfcand_cov_d0d0->clear();
  pfcand_cov_z0z0->clear();
  pfcand_cov_d0z0->clear();
  pfcand_cov_phid0->clear();
  pfcand_cov_tanLambdaz0->clear();
  pfcand_cov_d0omega->clear();
  pfcand_cov_d0tanLambda->clear();
  pfcand_cov_phiomega->clear();
  pfcand_cov_phiz0->clear();
  pfcand_cov_phitanLambda->clear();
  pfcand_cov_omegaz0->clear();
  pfcand_cov_omegatanLambda->clear();
  pfcand_d0->clear();
  pfcand_z0->clear();
  pfcand_Sip2dVal->clear();
  pfcand_Sip2dSig->clear();
  pfcand_Sip3dVal->clear();
  pfcand_Sip3dSig->clear();
  pfcand_JetDistVal->clear();
  pfcand_JetDistSig->clear();

  return;
}

StatusCode JetPIDRetriever::finalize() {
  if (Gaudi::Algorithm::finalize().isFailure()) return StatusCode::FAILURE;

  return StatusCode::SUCCESS;
}