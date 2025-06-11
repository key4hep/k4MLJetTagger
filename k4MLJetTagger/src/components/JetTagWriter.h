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
#ifndef JETTAGWRITER_H
#define JETTAGWRITER_H

#include "Gaudi/Algorithm.h"
#include "GaudiKernel/ITHistSvc.h"
#include "k4FWCore/DataHandle.h"
#include "k4FWCore/MetaDataHandle.h"

#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/ReconstructedParticleCollection.h>
#include <edm4hep/VertexCollection.h>

#include "TGraph.h"
#include "TH1F.h"

#include <edm4hep/ParticleIDCollection.h>
#include <edm4hep/utils/ParticleIDUtils.h>

/**
 * @class JetTagWriter
 * @brief This class is a Gaudi algorithm for writing jet PIDs to a TTree.
 *
 * The algorithm follows the Gaudi framework's lifecycle, with the initialize() method being called at the start,
 * execute() method being called for each event, and finalize() method being called at the end. The algorithm also
 * provides methods for initializing and cleaning the TTree.
 *
 * The execute function ...
 *
 * The output root file can be used for creating ROC curves to check the tagging performance.
 *
 * @author Sara Aumiller
 */
class JetTagWriter : public Gaudi::Algorithm {
public:
  /// Constructor.
  JetTagWriter(const std::string& name, ISvcLocator* svcLoc);
  /// Destructor.
  ~JetTagWriter() {};
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
  mutable k4FWCore::DataHandle<edm4hep::EventHeaderCollection> ev_handle{"EventHeader", Gaudi::DataHandle::Reader,
                                                                         this};
  mutable k4FWCore::DataHandle<edm4hep::ReconstructedParticleCollection> jets_handle{"RefinedVertexJets",
                                                                                     Gaudi::DataHandle::Reader, this};
  mutable k4FWCore::DataHandle<edm4hep::ParticleIDCollection> reco_jettag_G_handle{"RefinedJetTag_G",
                                                                                   Gaudi::DataHandle::Reader, this};
  mutable k4FWCore::DataHandle<edm4hep::ParticleIDCollection> reco_jettag_U_handle{"RefinedJetTag_U",
                                                                                   Gaudi::DataHandle::Reader, this};
  mutable k4FWCore::DataHandle<edm4hep::ParticleIDCollection> reco_jettag_D_handle{"RefinedJetTag_D",
                                                                                   Gaudi::DataHandle::Reader, this};
  mutable k4FWCore::DataHandle<edm4hep::ParticleIDCollection> reco_jettag_S_handle{"RefinedJetTag_S",
                                                                                   Gaudi::DataHandle::Reader, this};
  mutable k4FWCore::DataHandle<edm4hep::ParticleIDCollection> reco_jettag_C_handle{"RefinedJetTag_C",
                                                                                   Gaudi::DataHandle::Reader, this};
  mutable k4FWCore::DataHandle<edm4hep::ParticleIDCollection> reco_jettag_B_handle{"RefinedJetTag_B",
                                                                                   Gaudi::DataHandle::Reader, this};
  mutable k4FWCore::DataHandle<edm4hep::ParticleIDCollection> reco_jettag_TAU_handle{"RefinedJetTag_TAU",
                                                                                     Gaudi::DataHandle::Reader, this};
  mutable k4FWCore::DataHandle<edm4hep::ParticleIDCollection> mc_jettag_handle{"MCJetTag", Gaudi::DataHandle::Reader,
                                                                               this};

  SmartIF<ITHistSvc> m_ths; ///< THistogram service

  mutable TTree* t_jettag{nullptr};

  mutable bool recojet_isG;
  mutable float score_recojet_isG;
  mutable bool recojet_isU;
  mutable float score_recojet_isU;
  mutable bool recojet_isD;
  mutable float score_recojet_isD;
  mutable bool recojet_isS;
  mutable float score_recojet_isS;
  mutable bool recojet_isC;
  mutable float score_recojet_isC;
  mutable bool recojet_isB;
  mutable float score_recojet_isB;
  mutable bool recojet_isTAU;
  mutable float score_recojet_isTAU;

  mutable std::int32_t evNum;
};

#endif // JETTAGWRITER_H
