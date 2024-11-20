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

#include "edm4hep/MCParticleCollection.h"
#include "k4FWCore/Transformer.h"

#include <string>

struct ExampleTransformer final
    : k4FWCore::Transformer<edm4hep::MCParticleCollection(const edm4hep::MCParticleCollection&)> {
  ExampleTransformer(const std::string& name, ISvcLocator* svcLoc)
      : Transformer(name, svcLoc, {KeyValues("ExampleTransformerInputLocation", {"/InputExampleInt"})},
                    {KeyValues("ExampleTransformerOutputLocation", {"/OutputExampleInt"})}) {}

  edm4hep::MCParticleCollection operator()(const edm4hep::MCParticleCollection& input) const override {
    info() << "ExampleInt = " << input << endmsg;
    auto out = edm4hep::MCParticleCollection();
    for (const auto& mc : input) {
      out.push_back(mc);
    }
    return out;
  }
};

DECLARE_COMPONENT(ExampleTransformer)
