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
#include "k4FWCore/Transformer.h"
#include "k4FWCore/BaseClass.h"

// EDM4HEP
#include <edm4hep/ParticleIDCollection.h>
#include <edm4hep/ReconstructedParticleCollection.h>



// struct JetTagger final
//      : k4FWCore::Transformer<edm4hep::ParticleIDCollection(const edm4hep::ReconstructedParticleCollection&), BaseClass_t> {
//  
//    JetTagger(const std::string& name, ISvcLocator* svcLoc);
//    edm4hep::ParticleIDCollection operator()(const edm4hep::ReconstructedParticleCollection& input) const final; //override;
// };
