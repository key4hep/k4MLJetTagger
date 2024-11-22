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

// Note that in most cases it's better to use the functional
// approach than Gaudi::Algorithm

// GAUDI
#include "Gaudi/Algorithm.h"
#include "Gaudi/Property.h"

// K4FWCore
#include "k4FWCore/DataHandle.h"

// EDM4HEP
#include "edm4hep/ReconstructedParticleCollection.h"
// #include "edm4hep/ReconstructedParticle.h"
//#include "edm4hep/TrackCollection.h"
//#if __has_include("edm4hep/TrackerHit3DCollection.h")
//#include "edm4hep/TrackerHit3DCollection.h"
//#else
//#include "edm4hep/TrackerHitCollection.h"
//namespace edm4hep {
//  using TrackerHit3DCollection = edm4hep::TrackerHitCollection;
//}  // namespace edm4hep
//#endif

class JetTaggingAlg : public Gaudi::Algorithm {
public:
  JetTaggingAlg(const std::string&, ISvcLocator*);
  virtual ~JetTaggingAlg();
  /**  Initialize.
   *   @return status code
   */
  StatusCode initialize() override;
  /**  Execute.
   *   @return status code
   */
  StatusCode execute(const EventContext&) const override;
  /**  Finalize.
   *   @return status code
   */
  StatusCode finalize() override;

// include the collection needed here!!
private:
  // member variable
  Gaudi::Property<std::string> theMessage{this, "PerEventPrintMessage", "Hello ", "The message to printed for each Event"};
  mutable DataHandle<edm4hep::ReconstructedParticle> m_refinedVertexJets{"RefinedVertexJets", Gaudi::DataHandle::Reader, this};
  //mutable DataHandle<edm4hep::TrackerHit3DCollection> m_input_hits{"inputHits", Gaudi::DataHandle::Reader, this};

};
