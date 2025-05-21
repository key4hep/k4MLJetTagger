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

#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/ReconstructedParticleCollection.h>
#include <edm4hep/VertexCollection.h>

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
  ~JetTagWriter(){};
  /// Initialize.
  StatusCode initialize() override;
  /// Execute function.
  StatusCode execute(const EventContext&) const override;
  /// Finalize.
  StatusCode finalize() override;

private:
  /// Initialize tree.
  void initializeTree();
  /// Clean tree.
  void cleanTree() const;

  // Mark this algorithm as non-thread safe
  bool isReEntrant() const final { return false; }

  mutable DataHandle<edm4hep::EventHeaderCollection> m_eventHeaderHandle{"EventHeader", Gaudi::DataHandle::Reader,
                                                                         this};
  mutable DataHandle<edm4hep::ReconstructedParticleCollection> m_jetsHandle{"RefinedVertexJets",
                                                                            Gaudi::DataHandle::Reader, this};
  mutable DataHandle<edm4hep::ParticleIDCollection> m_recoJettagGHandle{"RefinedJetTag_G", Gaudi::DataHandle::Reader,
                                                                        this};
  mutable DataHandle<edm4hep::ParticleIDCollection> m_recoJettagUHandle{"RefinedJetTag_U", Gaudi::DataHandle::Reader,
                                                                        this};
  mutable DataHandle<edm4hep::ParticleIDCollection> m_recoJettagDHandle{"RefinedJetTag_D", Gaudi::DataHandle::Reader,
                                                                        this};
  mutable DataHandle<edm4hep::ParticleIDCollection> m_recoJettagSHandle{"RefinedJetTag_S", Gaudi::DataHandle::Reader,
                                                                        this};
  mutable DataHandle<edm4hep::ParticleIDCollection> m_recoJettagCHandle{"RefinedJetTag_C", Gaudi::DataHandle::Reader,
                                                                        this};
  mutable DataHandle<edm4hep::ParticleIDCollection> m_recoJettagBHandle{"RefinedJetTag_B", Gaudi::DataHandle::Reader,
                                                                        this};
  mutable DataHandle<edm4hep::ParticleIDCollection> m_recoJettagTauHandle{"RefinedJetTag_TAU",
                                                                          Gaudi::DataHandle::Reader, this};
  mutable DataHandle<edm4hep::ParticleIDCollection> m_mcJettagHandle{"MCJetTag", Gaudi::DataHandle::Reader, this};

  SmartIF<ITHistSvc> m_ths; ///< THistogram service

  mutable TTree* m_jettag{nullptr};

  mutable bool m_recojetIsG;
  mutable float m_scoreRecojetIsG;
  mutable bool m_recoJetIsU;
  mutable float m_scoreRecoJetIsU;
  mutable bool m_recoJetIsD;
  mutable float m_scoreRecoJetIsD;
  mutable bool m_recoJetIsS;
  mutable float m_scoreRecoJetIsS;
  mutable bool m_recoJetIsC;
  mutable float m_scoreRecoJetIsC;
  mutable bool m_recoJetIsB;
  mutable float m_scoreRecoJetIsB;
  mutable bool m_recoJetIsTAU;
  mutable float m_scoreRecoJetIsTau;

  mutable std::int32_t m_evNum;
};

#endif // JETTAGWRITER_H
