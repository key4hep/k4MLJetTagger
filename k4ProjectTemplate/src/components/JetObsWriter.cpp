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
#include "edm4hep/MCParticleCollection.h"
#include <edm4hep/VertexCollection.h>


#include <nlohmann/json.hpp> // Include a JSON parsing library
#include <fstream>

#include "Structs.h"
#include "JetObservablesRetriever.h"
#include "JetObsWriter.h"
#include "Helpers.h"
#include "DebugHelpers.h"

DECLARE_COMPONENT(JetObsWriter)

JetObsWriter::JetObsWriter(const std::string& name, ISvcLocator* svcLoc) : Gaudi::Algorithm(name, svcLoc) {
  declareProperty("InputJets", inputJets_handle, "Collection for input Jets");
  declareProperty("InputPrimaryVertices", inputPrimaryVertices_handle, "Collection for input Primary Vertices");
  declareProperty("InputMCParticles", inputMCParticles_handle, "Collection for input MC Particles");
}

StatusCode JetObsWriter::initialize() {
  if (Gaudi::Algorithm::initialize().isFailure()) return StatusCode::FAILURE;

  m_ths = service("THistSvc", true);
  if (!m_ths) {
    error() << "Couldn't get THistSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  t_jetcst = new TTree ("JetConstituentObservables", "Jet-Constituent Observables");
  if (m_ths->regTree("/rec/jetconst", t_jetcst).isFailure()) {
    error() << "Couldn't register jet constituent tree" << endmsg;
    return StatusCode::FAILURE;
  }

  initializeTree();

  return StatusCode::SUCCESS;
}

StatusCode JetObsWriter::execute(const EventContext&) const {

  auto evs = ev_handle.get();
  evNum = (*evs)[0].getEventNumber();
  //evNum = 0;
  info() << "Event number = " << evNum << endmsg;

  // Get the pointers to the collections
  const edm4hep::ReconstructedParticleCollection *jet_coll_ptr = inputJets_handle.get();
  const edm4hep::VertexCollection *prim_vertex_coll_ptr = inputPrimaryVertices_handle.get();
  const edm4hep::MCParticleCollection *mc_coll_ptr = inputMCParticles_handle.get();
  // Create references to the collections
  const edm4hep::ReconstructedParticleCollection &jet_coll = *jet_coll_ptr;
  const edm4hep::VertexCollection &prim_vertex_coll = *prim_vertex_coll_ptr;
  const edm4hep::MCParticleCollection &mc_coll = *mc_coll_ptr;

  JetObservablesRetriever Retriever;
  Retriever.Bz = 2.0; // hardcoded for now

  for (const auto& jet : jet_coll) { // loop over all jets in the event
    cleanTree();
    Jet j = Retriever.retrieve_input_observables(jet, prim_vertex_coll); // get all observables
    for (const auto& pfc : j.constituents) { // loop over all jet constituents / pfcands
      pfcand_erel_log->push_back(pfc.pfcand_erel_log);
      pfcand_thetarel->push_back(pfc.pfcand_thetarel);
      pfcand_phirel->push_back(pfc.pfcand_phirel);
      pfcand_e->push_back(pfc.pfcand_e);
      pfcand_p->push_back(pfc.pfcand_p);
      pfcand_type->push_back(pfc.pfcand_type);
      pfcand_charge->push_back(pfc.pfcand_charge);
      pfcand_isEl->push_back(pfc.pfcand_isEl);
      pfcand_isMu->push_back(pfc.pfcand_isMu);
      pfcand_isGamma->push_back(pfc.pfcand_isGamma);
      pfcand_isChargedHad->push_back(pfc.pfcand_isChargedHad);
      pfcand_isNeutralHad->push_back(pfc.pfcand_isNeutralHad);
      pfcand_dndx->push_back(pfc.pfcand_dndx);
      pfcand_tof->push_back(pfc.pfcand_tof);
      pfcand_cov_omegaomega->push_back(pfc.pfcand_cov_omegaomega);
      pfcand_cov_tanLambdatanLambda->push_back(pfc.pfcand_cov_tanLambdatanLambda);
      pfcand_cov_phiphi->push_back(pfc.pfcand_cov_phiphi);
      pfcand_cov_d0d0->push_back(pfc.pfcand_cov_d0d0);
      pfcand_cov_z0z0->push_back(pfc.pfcand_cov_z0z0);
      pfcand_cov_d0z0->push_back(pfc.pfcand_cov_d0z0);
      pfcand_cov_phid0->push_back(pfc.pfcand_cov_phid0);
      pfcand_cov_tanLambdaz0->push_back(pfc.pfcand_cov_tanLambdaz0);
      pfcand_cov_d0omega->push_back(pfc.pfcand_cov_d0omega);
      pfcand_cov_d0tanLambda->push_back(pfc.pfcand_cov_d0tanLambda);
      pfcand_cov_phiomega->push_back(pfc.pfcand_cov_phiomega);
      pfcand_cov_phiz0->push_back(pfc.pfcand_cov_phiz0);
      pfcand_cov_phitanLambda->push_back(pfc.pfcand_cov_phitanLambda);
      pfcand_cov_omegaz0->push_back(pfc.pfcand_cov_omegaz0);
      pfcand_cov_omegatanLambda->push_back(pfc.pfcand_cov_omegatanLambda);
      pfcand_d0->push_back(pfc.pfcand_d0);
      pfcand_z0->push_back(pfc.pfcand_z0);
      pfcand_Sip2dVal->push_back(pfc.pfcand_Sip2dVal);
      pfcand_Sip2dSig->push_back(pfc.pfcand_Sip2dSig);
      pfcand_Sip3dVal->push_back(pfc.pfcand_Sip3dVal);
      pfcand_Sip3dSig->push_back(pfc.pfcand_Sip3dSig);
      pfcand_JetDistVal->push_back(pfc.pfcand_JetDistVal);
      pfcand_JetDistSig->push_back(pfc.pfcand_JetDistSig);
    }
    // PV variables
    const edm4hep::Vector3f prim_vertex = Retriever.get_primary_vertex(prim_vertex_coll);
    jet_PV_x = prim_vertex.x;
    jet_PV_y = prim_vertex.y;
    jet_PV_z = prim_vertex.z;
    jet_PV_id = evNum;
    // MC PV variables
    const edm4hep::MCParticle MCquark = get_MC_quark(mc_coll);
    const edm4hep::Vector3d MCprim_vertex = MCquark.getVertex();
    jet_MCPV_x = MCprim_vertex.x;
    jet_MCPV_y = MCprim_vertex.y;
    jet_MCPV_z = MCprim_vertex.z;

    
    t_jetcst->Fill();
  }

  return StatusCode::SUCCESS;
}

void JetObsWriter::initializeTree() {

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

  // PV variables
  t_jetcst->Branch("jet_PV_x", &jet_PV_x);
  t_jetcst->Branch("jet_PV_y", &jet_PV_y);
  t_jetcst->Branch("jet_PV_z", &jet_PV_z);
  t_jetcst->Branch("jet_PV_id", &jet_PV_id);
  t_jetcst->Branch("jet_MCPV_x", &jet_MCPV_x);
  t_jetcst->Branch("jet_MCPV_y", &jet_MCPV_y);
  t_jetcst->Branch("jet_MCPV_z", &jet_MCPV_z);

  return;
}

void JetObsWriter::cleanTree() const {
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

  float dummy_value = -999.0;
  jet_PV_x = dummy_value;
  jet_PV_y = dummy_value;
  jet_PV_z = dummy_value;
  jet_PV_id = -1;
  jet_MCPV_x = dummy_value;
  jet_MCPV_y = dummy_value;
  jet_MCPV_z = dummy_value;

  return;
}

StatusCode JetObsWriter::finalize() {
  if (Gaudi::Algorithm::finalize().isFailure()) return StatusCode::FAILURE;

  return StatusCode::SUCCESS;
}