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

// #include "JetTagger.h"

#include "GAUDI_VERSION.h"
#include "GaudiKernel/MsgStream.h"
#include "k4FWCore/Transformer.h"
#include <edm4hep/ParticleIDCollection.h>
#include <edm4hep/ReconstructedParticleCollection.h>

#include <random>

// Declare the function at the top
int fancyModelDoTag(const edm4hep::ReconstructedParticle& jet, MsgStream& log);

// main function
struct JetTagger //final
    : k4FWCore::Transformer<edm4hep::ParticleIDCollection(const edm4hep::ReconstructedParticleCollection&)> {
  JetTagger(const std::string& name, ISvcLocator* svcLoc)
    : Transformer(name, svcLoc, 
                  {KeyValues("InputJets", {"RefinedVertexJets"})},
                  {KeyValues("OutputIDCollections", {"RefinedJetTags"})}
                  ) {}

  edm4hep::ParticleIDCollection operator()(const edm4hep::ReconstructedParticleCollection& inputJets) const override{
    info() << "Tagging " << inputJets.size() << " input jets" << endmsg;
    auto tagCollection = edm4hep::ParticleIDCollection();

    for (const auto& jet : inputJets) {
      auto tagValue = fancyModelDoTag(jet, info());  // this is where you will have to
                                             // put in an actual thing
      auto jetTag = tagCollection.create();
      jetTag.setParticle(jet);
      jetTag.setType(tagValue);  // for example
    }

    return tagCollection;
  };
};

// Function that simulates a tagging model by returning a dummy value.
int fancyModelDoTag(const edm4hep::ReconstructedParticle& jet, MsgStream& log) {
    // Create a random number generator for demonstration purposes.
    static std::mt19937 rng(42);  // Seed for reproducibility
    static std::uniform_int_distribution<int> dist(0, 6);  // Assume 7 jet flavors: 0 to 6

    // Generate a random tag value for the jet.
    int tagValue = dist(rng);

    log << "Tagged jet with energy " << jet.getEnergy() << endmsg;
    log << "Tag value: " << tagValue << endmsg;

    return tagValue;
}

DECLARE_COMPONENT(JetTagger)
