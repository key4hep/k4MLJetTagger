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
#ifndef JETOBSWRITER_H
#define JETOBSWRITER_H

#include "Gaudi/Algorithm.h"
#include "GaudiKernel/ITHistSvc.h"
#include "k4FWCore/DataHandle.h"

#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/ReconstructedParticleCollection.h>
#include <edm4hep/VertexCollection.h>

#include "JetObservablesRetriever.h"

/**
 * @class JetObsWriter
 * @brief This class is a Gaudi algorithm for writing jet observables that are used for tagging to a TTree.
 *
 * The algorithm follows the Gaudi framework's lifecycle, with the initialize() method being called at the start,
 * execute() method being called for each event, and finalize() method being called at the end. The algorithm also
 * provides methods for initializing and cleaning the TTree.
 *
 * The execute function loops over all jets in the events and retrieves the observables for tagging with the
 * JetObservablesRetriever. It then dumps all the information into a TTree. The output root file can be used for
 * training a neural network for jet tagging.
 *
 * @note The naming convention for the observables follows the key4hep implementation (see Structs.h and for the
 * conversion to the old FCCAnalyses convention Helpers.cpp).
 *
 * @author Sara Aumiller
 */
class JetObsWriter : public Gaudi::Algorithm {
public:
  /// Constructor.
  JetObsWriter(const std::string& name, ISvcLocator* svcLoc);
  /// Destructor.
  ~JetObsWriter() {
    delete m_pfcandErelLog;
    delete m_pfcandThetarel;
    delete m_pfcandPhirel;
    delete m_pfcandE;
    delete m_pfcandP;
    delete m_pfcandType;
    delete m_pfcandCharge;
    delete m_pfcandIsEl;
    delete m_pfcandIsMu;
    delete m_pfcandIsGamma;
    delete m_pfcandIsChargedHad;
    delete m_pfcandIsNeutralHad;
    delete m_pfcandDndx;
    delete m_pfcandTof;
    delete m_pfcandCovOmegaOmega;
    delete m_pfcandCovTanLambdaTanLambda;
    delete m_pfcandCovPhiPhi;
    delete m_pfcandCovD0D0;
    delete m_pfcandCovZ0Z0;
    delete m_pfcandCovD0Z0;
    delete m_pfcandCovPhiD0;
    delete m_pfcandCovTanLambdaZ0;
    delete m_pfcandCovD0Omega;
    delete m_pfcandCovD0TanLambda;
    delete m_pfcandCovPhiOmega;
    delete m_pfcandCovPhiZ0;
    delete m_pfcandCovPhiTanLambda;
    delete m_pfcandCovOmegaZ0;
    delete m_pfcandCovOmegaTanLambda;
    delete m_pfcandD0;
    delete m_pfcandZ0;
    delete m_pfcandSip2dVal;
    delete m_pfcandSip2dSig;
    delete m_pfcandSip3dVal;
    delete m_pfcandSip3dSig;
    delete m_pfcandJetDistVal;
    delete m_pfcandJetDistSig;
  };
  /// Initialize.
  virtual StatusCode initialize();
  /// Initialize tree.
  void initializeTree();
  /// Clean tree.
  void cleanTree() const;
  /// Execute function.
  virtual StatusCode execute(const EventContext&) const;
  /// Finalize.
  virtual StatusCode finalize();

private:
  mutable k4FWCore::DataHandle<edm4hep::EventHeaderCollection> m_eventHeaderHandle{"EventHeader", Gaudi::DataHandle::Reader,
                                                                                   this};
  mutable k4FWCore::DataHandle<edm4hep::ReconstructedParticleCollection> m_inputJetsHandle{"InputJets", Gaudi::DataHandle::Reader,
                                                                                           this};
  mutable k4FWCore::DataHandle<edm4hep::VertexCollection> m_inputPrimaryVerticesHandle{"InputPrimaryVertices",
                                                                                      Gaudi::DataHandle::Reader, this};

  mutable JetObservablesRetriever* m_retriever;

  SmartIF<ITHistSvc> m_ths; ///< THistogram service

  mutable TTree* m_jetcst{nullptr};

  mutable std::vector<float>* m_pfcandErelLog = nullptr;
  mutable std::vector<float>* m_pfcandThetarel = nullptr;
  mutable std::vector<float>* m_pfcandPhirel = nullptr;
  mutable std::vector<float>* m_pfcandE = nullptr;
  mutable std::vector<float>* m_pfcandP = nullptr;
  mutable std::vector<int>* m_pfcandType = nullptr;
  mutable std::vector<int>* m_pfcandCharge = nullptr;
  mutable std::vector<int>* m_pfcandIsEl = nullptr;
  mutable std::vector<int>* m_pfcandIsMu = nullptr;
  mutable std::vector<int>* m_pfcandIsGamma = nullptr;
  mutable std::vector<int>* m_pfcandIsChargedHad = nullptr;
  mutable std::vector<int>* m_pfcandIsNeutralHad = nullptr;
  mutable std::vector<float>* m_pfcandDndx = nullptr;
  mutable std::vector<float>* m_pfcandTof = nullptr;
  mutable std::vector<float>* m_pfcandCovOmegaOmega = nullptr;
  mutable std::vector<float>* m_pfcandCovTanLambdaTanLambda = nullptr;
  mutable std::vector<float>* m_pfcandCovPhiPhi = nullptr;
  mutable std::vector<float>* m_pfcandCovD0D0 = nullptr;
  mutable std::vector<float>* m_pfcandCovZ0Z0 = nullptr;
  mutable std::vector<float>* m_pfcandCovD0Z0 = nullptr;
  mutable std::vector<float>* m_pfcandCovPhiD0 = nullptr;
  mutable std::vector<float>* m_pfcandCovTanLambdaZ0 = nullptr;
  mutable std::vector<float>* m_pfcandCovD0Omega = nullptr;
  mutable std::vector<float>* m_pfcandCovD0TanLambda = nullptr;
  mutable std::vector<float>* m_pfcandCovPhiOmega = nullptr;
  mutable std::vector<float>* m_pfcandCovPhiZ0 = nullptr;
  mutable std::vector<float>* m_pfcandCovPhiTanLambda = nullptr;
  mutable std::vector<float>* m_pfcandCovOmegaZ0 = nullptr;
  mutable std::vector<float>* m_pfcandCovOmegaTanLambda = nullptr;
  mutable std::vector<float>* m_pfcandD0 = nullptr;
  mutable std::vector<float>* m_pfcandZ0 = nullptr;
  mutable std::vector<float>* m_pfcandSip2dVal = nullptr;
  mutable std::vector<float>* m_pfcandSip2dSig = nullptr;
  mutable std::vector<float>* m_pfcandSip3dVal = nullptr;
  mutable std::vector<float>* m_pfcandSip3dSig = nullptr;
  mutable std::vector<float>* m_pfcandJetDistVal = nullptr;
  mutable std::vector<float>* m_pfcandJetDistSig = nullptr;
  // Not input to network but good to check:
  mutable float m_jetPVx;
  mutable float m_jetPVy;
  mutable float m_jetPVz;
  mutable int m_jetPVid;

  mutable std::int32_t m_evNum;
};

#endif // JETOBSWRITER_H
