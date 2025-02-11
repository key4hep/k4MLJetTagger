#include "DebugHelpers.h"
#include <set>

edm4hep::MCParticle get_MC_quark(const edm4hep::MCParticleCollection& mc_coll){
    edm4hep::MCParticle quark;
    int higgs_pid = 25;
    std::set<int> expected_flavors = {1, 2, 3, 4, 5, 15, 21};  // u, d, s, c, b, tau, g
    for (const auto& mc_part : mc_coll) {
        if(mc_part.getPDG() == higgs_pid){
            // // const edm4hep::Vector3d higgs_vertex = mc_part.getVertex();
            // // std::cout << "Higgs vertex: " << higgs_vertex.x << ", " << higgs_vertex.y << ", " << higgs_vertex.z << std::endl;
            // auto daughters = mc_part.getDaughters();
            // if (daughters.size() != 2){
            //     throw std::invalid_argument("Higgs has " + std::to_string(daughters.size()) + " daughters. Expected 2.");
            // }
            // // check if daughters are the expected quarks
            // for (const auto& daughter : daughters){
            //     if (expected_flavors.find(std::abs(daughter.getPDG())) == expected_flavors.end()){ // if not found
            //         throw std::invalid_argument("Higgs daughter has unexpected PID: " + std::to_string(daughter.getPDG()));
            //     } 
            //     // const edm4hep::Vector3d vertex = daughter.getVertex();
            //     // std::cout << "Daughter vertex: " << vertex.x << ", " << vertex.y << ", " << vertex.z << std::endl;
            // }
            // // just take the first one, vertices are the same
            // quark = daughters[0];
            quark = mc_part;
        }
    };
    return quark;
};