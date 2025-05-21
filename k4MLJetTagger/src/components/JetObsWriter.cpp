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
#include <edm4hep/ParticleIDCollection.h>
#include <edm4hep/ReconstructedParticleCollection.h>
#include <edm4hep/VertexCollection.h>

#include "JetObsWriter.h"
#include "Structs.h"

DECLARE_COMPONENT(JetObsWriter)

JetObsWriter::JetObsWriter(const std::string& name, ISvcLocator* svcLoc) : Gaudi::Algorithm(name, svcLoc) {
  declareProperty("InputJets", m_inputJetsHandle, "Collection for input Jets");
  declareProperty("InputPrimaryVertices", m_inputPrimaryVerticesHandle, "Collection for input Primary Vertices");
}

StatusCode JetObsWriter::initialize() {
  if (Gaudi::Algorithm::initialize().isFailure())
    return StatusCode::FAILURE;

  m_ths = service("THistSvc", true);
  if (!m_ths) {
    error() << "Couldn't get THistSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  m_jetcst = new TTree("JetConstituentObservables", "Jet-Constituent Observables");
  if (m_ths->regTree("/rec/jetconst", m_jetcst).isFailure()) {
    error() << "Couldn't register jet constituent tree" << endmsg;
    return StatusCode::FAILURE;
  }

  initializeTree();

  // JetObservablesRetriever object
  m_retriever = new JetObservablesRetriever();
  m_retriever->Bz = 2.0; // hardcoded for now

  return StatusCode::SUCCESS;
}

StatusCode JetObsWriter::execute(const EventContext&) const {
  auto evs = m_eventHeaderHandle.get();
  m_evNum = (*evs)[0].getEventNumber();
  // evNum = 0;
  info() << "Event number = " << m_evNum << endmsg;

  // Get the pointers to the collections
  const edm4hep::ReconstructedParticleCollection* jet_coll_ptr = m_inputJetsHandle.get();
  const edm4hep::VertexCollection* prim_vertex_coll_ptr = m_inputPrimaryVerticesHandle.get();
  // Create references to the collections
  const edm4hep::ReconstructedParticleCollection& jet_coll = *jet_coll_ptr;
  const edm4hep::VertexCollection& prim_vertex_coll = *prim_vertex_coll_ptr;

  for (const auto& jet : jet_coll) { // loop over all jets in the event
    cleanTree();
    Jet j = m_retriever->retrieve_input_observables(jet, prim_vertex_coll); // get all observables
    for (const auto& pfc : j.constituents) {                                // loop over all jet constituents / pfcands
      m_pfcandErelLog->push_back(pfc.pfcand_erel_log);
      m_pfcandThetarel->push_back(pfc.pfcand_thetarel);
      m_pfcandPhirel->push_back(pfc.pfcand_phirel);
      m_pfcandE->push_back(pfc.pfcand_e);
      m_pfcandP->push_back(pfc.pfcand_p);
      m_pfcandType->push_back(pfc.pfcand_type);
      m_pfcandCharge->push_back(pfc.pfcand_charge);
      m_pfcandIsEl->push_back(pfc.pfcand_isEl);
      m_pfcandIsMu->push_back(pfc.pfcand_isMu);
      m_pfcandIsGamma->push_back(pfc.pfcand_isGamma);
      m_pfcandIsChargedHad->push_back(pfc.pfcand_isChargedHad);
      m_pfcandIsNeutralHad->push_back(pfc.pfcand_isNeutralHad);
      m_pfcandDndx->push_back(pfc.pfcand_dndx);
      m_pfcandTof->push_back(pfc.pfcand_tof);
      m_pfcandCovOmegaOmega->push_back(pfc.pfcand_cov_omegaomega);
      m_pfcandCovTanLambdaTanLambda->push_back(pfc.pfcand_cov_tanLambdatanLambda);
      m_pfcandCovPhiPhi->push_back(pfc.pfcand_cov_phiphi);
      m_pfcandCovD0D0->push_back(pfc.pfcand_cov_d0d0);
      m_pfcandCovZ0Z0->push_back(pfc.pfcand_cov_z0z0);
      m_pfcandCovD0Z0->push_back(pfc.pfcand_cov_d0z0);
      m_pfcandCovPhiD0->push_back(pfc.pfcand_cov_phid0);
      m_pfcandCovTanLambdaZ0->push_back(pfc.pfcand_cov_tanLambdaz0);
      m_pfcandCovD0Omega->push_back(pfc.pfcand_cov_d0omega);
      m_pfcandCovD0TanLambda->push_back(pfc.pfcand_cov_d0tanLambda);
      m_pfcandCovPhiOmega->push_back(pfc.pfcand_cov_phiomega);
      m_pfcandCovPhiZ0->push_back(pfc.pfcand_cov_phiz0);
      m_pfcandCovPhiTanLambda->push_back(pfc.pfcand_cov_phitanLambda);
      m_pfcandCovOmegaZ0->push_back(pfc.pfcand_cov_omegaz0);
      m_pfcandCovOmegaTanLambda->push_back(pfc.pfcand_cov_omegatanLambda);
      m_pfcandD0->push_back(pfc.pfcand_d0);
      m_pfcandZ0->push_back(pfc.pfcand_z0);
      m_pfcandSip2dVal->push_back(pfc.pfcand_Sip2dVal);
      m_pfcandSip2dSig->push_back(pfc.pfcand_Sip2dSig);
      m_pfcandSip3dVal->push_back(pfc.pfcand_Sip3dVal);
      m_pfcandSip3dSig->push_back(pfc.pfcand_Sip3dSig);
      m_pfcandJetDistVal->push_back(pfc.pfcand_JetDistVal);
      m_pfcandJetDistSig->push_back(pfc.pfcand_JetDistSig);
    }
    // PV variables
    const edm4hep::Vector3f prim_vertex = m_retriever->get_primary_vertex(prim_vertex_coll);
    m_jetPVx = prim_vertex.x;
    m_jetPVy = prim_vertex.y;
    m_jetPVz = prim_vertex.z;

    m_jetcst->Fill();
  }

  return StatusCode::SUCCESS;
}

void JetObsWriter::initializeTree() {
  m_pfcandErelLog = new std::vector<float>();
  m_pfcandThetarel = new std::vector<float>();
  m_pfcandPhirel = new std::vector<float>();
  m_pfcandE = new std::vector<float>();
  m_pfcandP = new std::vector<float>();
  m_pfcandType = new std::vector<int>();
  m_pfcandCharge = new std::vector<int>();
  m_pfcandIsEl = new std::vector<int>();
  m_pfcandIsMu = new std::vector<int>();
  m_pfcandIsGamma = new std::vector<int>();
  m_pfcandIsChargedHad = new std::vector<int>();
  m_pfcandIsNeutralHad = new std::vector<int>();
  m_pfcandDndx = new std::vector<float>();
  m_pfcandTof = new std::vector<float>();
  m_pfcandCovOmegaOmega = new std::vector<float>();
  m_pfcandCovTanLambdaTanLambda = new std::vector<float>();
  m_pfcandCovPhiPhi = new std::vector<float>();
  m_pfcandCovD0D0 = new std::vector<float>();
  m_pfcandCovZ0Z0 = new std::vector<float>();
  m_pfcandCovD0Z0 = new std::vector<float>();
  m_pfcandCovPhiD0 = new std::vector<float>();
  m_pfcandCovTanLambdaZ0 = new std::vector<float>();
  m_pfcandCovD0Omega = new std::vector<float>();
  m_pfcandCovD0TanLambda = new std::vector<float>();
  m_pfcandCovPhiOmega = new std::vector<float>();
  m_pfcandCovPhiZ0 = new std::vector<float>();
  m_pfcandCovPhiTanLambda = new std::vector<float>();
  m_pfcandCovOmegaZ0 = new std::vector<float>();
  m_pfcandCovOmegaTanLambda = new std::vector<float>();
  m_pfcandD0 = new std::vector<float>();
  m_pfcandZ0 = new std::vector<float>();
  m_pfcandSip2dVal = new std::vector<float>();
  m_pfcandSip2dSig = new std::vector<float>();
  m_pfcandSip3dVal = new std::vector<float>();
  m_pfcandSip3dSig = new std::vector<float>();
  m_pfcandJetDistVal = new std::vector<float>();
  m_pfcandJetDistSig = new std::vector<float>();

  m_jetcst->Branch("pfcand_erel_log", &m_pfcandErelLog);
  m_jetcst->Branch("pfcand_thetarel", &m_pfcandThetarel);
  m_jetcst->Branch("pfcand_phirel", &m_pfcandPhirel);
  m_jetcst->Branch("pfcand_e", &m_pfcandE);
  m_jetcst->Branch("pfcand_p", &m_pfcandP);
  m_jetcst->Branch("pfcand_type", &m_pfcandType);
  m_jetcst->Branch("pfcand_charge", &m_pfcandCharge);
  m_jetcst->Branch("pfcand_isEl", &m_pfcandIsEl);
  m_jetcst->Branch("pfcand_isMu", &m_pfcandIsMu);
  m_jetcst->Branch("pfcand_isGamma", &m_pfcandIsGamma);
  m_jetcst->Branch("pfcand_isChargedHad", &m_pfcandIsChargedHad);
  m_jetcst->Branch("pfcand_isNeutralHad", &m_pfcandIsNeutralHad);
  m_jetcst->Branch("pfcand_dndx", &m_pfcandDndx);
  m_jetcst->Branch("pfcand_tof", &m_pfcandTof);
  m_jetcst->Branch("pfcand_cov_omegaomega", &m_pfcandCovOmegaOmega);
  m_jetcst->Branch("pfcand_cov_tanLambdatanLambda", &m_pfcandCovTanLambdaTanLambda);
  m_jetcst->Branch("pfcand_cov_phiphi", &m_pfcandCovPhiPhi);
  m_jetcst->Branch("pfcand_cov_d0d0", &m_pfcandCovD0D0);
  m_jetcst->Branch("pfcand_cov_z0z0", &m_pfcandCovZ0Z0);
  m_jetcst->Branch("pfcand_cov_d0z0", &m_pfcandCovD0Z0);
  m_jetcst->Branch("pfcand_cov_phid0", &m_pfcandCovPhiD0);
  m_jetcst->Branch("pfcand_cov_tanLambdaz0", &m_pfcandCovTanLambdaZ0);
  m_jetcst->Branch("pfcand_cov_d0omega", &m_pfcandCovD0Omega);
  m_jetcst->Branch("pfcand_cov_d0tanLambda", &m_pfcandCovD0TanLambda);
  m_jetcst->Branch("pfcand_cov_phiomega", &m_pfcandCovPhiOmega);
  m_jetcst->Branch("pfcand_cov_phiz0", &m_pfcandCovPhiZ0);
  m_jetcst->Branch("pfcand_cov_phitanLambda", &m_pfcandCovPhiTanLambda);
  m_jetcst->Branch("pfcand_cov_omegaz0", &m_pfcandCovOmegaZ0);
  m_jetcst->Branch("pfcand_cov_omegatanLambda", &m_pfcandCovOmegaTanLambda);
  m_jetcst->Branch("pfcand_d0", &m_pfcandD0);
  m_jetcst->Branch("pfcand_z0", &m_pfcandZ0);
  m_jetcst->Branch("pfcand_Sip2dVal", &m_pfcandSip2dVal);
  m_jetcst->Branch("pfcand_Sip2dSig", &m_pfcandSip2dSig);
  m_jetcst->Branch("pfcand_Sip3dVal", &m_pfcandSip3dVal);
  m_jetcst->Branch("pfcand_Sip3dSig", &m_pfcandSip3dSig);
  m_jetcst->Branch("pfcand_JetDistVal", &m_pfcandJetDistVal);
  m_jetcst->Branch("pfcand_JetDistSig", &m_pfcandJetDistSig);

  // PV variables
  m_jetcst->Branch("jet_PV_x", &m_jetPVx);
  m_jetcst->Branch("jet_PV_y", &m_jetPVy);
  m_jetcst->Branch("jet_PV_z", &m_jetPVz);

  return;
}

void JetObsWriter::cleanTree() const {
  m_pfcandErelLog->clear();
  m_pfcandThetarel->clear();
  m_pfcandPhirel->clear();
  m_pfcandE->clear();
  m_pfcandP->clear();
  m_pfcandType->clear();
  m_pfcandCharge->clear();
  m_pfcandIsEl->clear();
  m_pfcandIsMu->clear();
  m_pfcandIsGamma->clear();
  m_pfcandIsChargedHad->clear();
  m_pfcandIsNeutralHad->clear();
  m_pfcandDndx->clear();
  m_pfcandTof->clear();
  m_pfcandCovOmegaOmega->clear();
  m_pfcandCovTanLambdaTanLambda->clear();
  m_pfcandCovPhiPhi->clear();
  m_pfcandCovD0D0->clear();
  m_pfcandCovZ0Z0->clear();
  m_pfcandCovD0Z0->clear();
  m_pfcandCovPhiD0->clear();
  m_pfcandCovTanLambdaZ0->clear();
  m_pfcandCovD0Omega->clear();
  m_pfcandCovD0TanLambda->clear();
  m_pfcandCovPhiOmega->clear();
  m_pfcandCovPhiZ0->clear();
  m_pfcandCovPhiTanLambda->clear();
  m_pfcandCovOmegaZ0->clear();
  m_pfcandCovOmegaTanLambda->clear();
  m_pfcandD0->clear();
  m_pfcandZ0->clear();
  m_pfcandSip2dVal->clear();
  m_pfcandSip2dSig->clear();
  m_pfcandSip3dVal->clear();
  m_pfcandSip3dSig->clear();
  m_pfcandJetDistVal->clear();
  m_pfcandJetDistSig->clear();

  float dummy_value = -999.0;
  m_jetPVx = dummy_value;
  m_jetPVy = dummy_value;
  m_jetPVz = dummy_value;

  return;
}

StatusCode JetObsWriter::finalize() {
  if (Gaudi::Algorithm::finalize().isFailure())
    return StatusCode::FAILURE;

  return StatusCode::SUCCESS;
}
