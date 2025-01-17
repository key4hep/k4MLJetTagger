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

#include "edm4hep/MCParticleCollection.h"
#include <edm4hep/ParticleIDCollection.h>
#include <edm4hep/ReconstructedParticleCollection.h>

#include <fstream>
#include <cmath> // For std::abs

#include "WeaverInterface.h"
#include "Helpers.h"


int findMCPIDfromHiggsDaughters(const edm4hep::MCParticleCollection& MCParticles, MsgStream& log) {
  // find the MC PID of the jet by looking at the MC Higgs Boson and finding its daughters
  // WARNING: This uses the assumption of H(jj)Z(vv) events!!!

  // find the MC Higgs Boson
  int HiggsPID = 25;
  std::set<int> expectedFlavors = {1, 2, 3, 4, 5, 15, 21}; // u,d,s,c,b,tau,g
  std::vector<int> HiggsDaughtersPDG;
  for (const auto& MCParticle : MCParticles) { // loop over all MC particles
    if (MCParticle.getPDG() == HiggsPID) { // find the Higgs 
      for (const auto& daughter : MCParticle.getDaughters()) {       // find the daughters of the Higgs Boson
        int daughterPID = daughter.getPDG();
        HiggsDaughtersPDG.push_back(daughterPID);
      }
    }
  }
  // check if the daughters are all the same (ignoring signs)
  if (HiggsDaughtersPDG.size() == 2 && HiggsDaughtersPDG[0] == -HiggsDaughtersPDG[1]){
    int j_pid = std::abs(HiggsDaughtersPDG[0]);
    if(expectedFlavors.count(j_pid) == 1){
      return j_pid;
    }
  } else {
    log << MSG::ERROR << "Higgs Boson has more than 2 daughters or they are not the same. Returning dummy value 0 for MC jet flavor." << endmsg;
  }
  return 0; // dummy value
}


/**
* @class JetMCPIDFinder
* @brief Gaudi transformer that attaches a edm4hep::ParticleIDCollection object "MCJetTag" to each jet in the input collection "RefinedVertexJets".
*
* WARNING: This uses the assumption of H(jj)Z(vv) events!!!
* The function loops over all jets in the input collection and attaches a ParticleID object to each jet by looking at the MC Higgs Boson and finding its daughters.
* In the future, the function "findMCPIDfromHiggsDaughters" should be replaced by a more general function that can be used for any event topology.
*
* @author Sara Aumiller
*/
struct JetMCPIDFinder
    : k4FWCore::Transformer<edm4hep::ParticleIDCollection(const edm4hep::ReconstructedParticleCollection&, const edm4hep::MCParticleCollection& )> {
  JetMCPIDFinder(const std::string& name, ISvcLocator* svcLoc)
    : Transformer(name, svcLoc, 
                  {
                    KeyValues("InputJets", {"RefinedVertexJets"}),
                    KeyValues("MCParticles", {"MCParticles"})
                  },
                  {KeyValues("OutputIDCollection", {"MCJetTag"})}
                  ) {}

  edm4hep::ParticleIDCollection operator()(const edm4hep::ReconstructedParticleCollection& inputJets, const edm4hep::MCParticleCollection& MCParticles) const override{
    // info() << "Finding MC PID of " << inputJets.size() << " input jets" << endmsg;
    warning() << "Fining the MC PID of jets uses the assumption of H(jj)Z(vv) events!!! " << endmsg;

    auto tagCollection = edm4hep::ParticleIDCollection();

    for (const auto& jet : inputJets) {
    
      int MCflavor = findMCPIDfromHiggsDaughters(MCParticles, msg());

      auto jetTag = tagCollection.create();
      jetTag.setParticle(jet);
      jetTag.setPDG(MCflavor);
    }

    return tagCollection;
  };
};

DECLARE_COMPONENT(JetMCPIDFinder)
