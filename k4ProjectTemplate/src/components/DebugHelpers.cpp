#include "DebugHelpers.h"
#include <set>

edm4hep::MCParticle get_MC_quark(const edm4hep::MCParticleCollection& mc_coll){
    edm4hep::MCParticle quark;
    int higgs_pid = 25;
    std::set<int> expected_flavors = {1, 2, 3, 4, 5, 15, 21};  // u, d, s, c, b, tau, g
    for (const auto& mc_part : mc_coll) {
        if(mc_part.getPDG() == higgs_pid){
            auto daughters = mc_part.getDaughters();
            if (daughters.size() != 2){
                throw std::invalid_argument("Higgs has " + std::to_string(daughters.size()) + " daughters. Expected 2.");
            }
            for (const auto& daughter : daughters){
                if (expected_flavors.find(std::abs(daughter.getPDG())) != expected_flavors.end()){
                    quark = daughter; // will be overwritten by the second daughter, but that's fine, doesn't matter which daughter we return
                } else {
                    throw std::invalid_argument("Higgs daughter has unexpected PID: " + std::to_string(daughter.getPDG()));
                }
            }
        }
    };
    return quark;
};