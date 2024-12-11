#include "VarMapper.h"

VarMapper::VarMapper() {
    // Initialize mappings
    map_to_FCCAn["pfcand_erel_log"] = "pfcand_erel_log";
    map_to_FCCAn["pfcand_thetarel"] = "pfcand_thetarel";
    map_to_FCCAn["pfcand_phirel"] = "pfcand_phirel";
    map_to_FCCAn["pfcand_cov_cc"] = "pfcand_dptdpt";
    map_to_FCCAn["pfcand_cov_tanLambdatanLambda"] = "pfcand_detadeta";
    map_to_FCCAn["pfcand_cov_phiphi"] = "pfcand_dphidphi";
    map_to_FCCAn["pfcand_cov_d0d0"] = "pfcand_dxydxy";
    map_to_FCCAn["pfcand_cov_z0z0"] = "pfcand_dzdz";
    map_to_FCCAn["pfcand_cov_d0z0"] = "pfcand_dxydz";
    map_to_FCCAn["pfcand_cov_phid0"] = "pfcand_dphidxy";
    map_to_FCCAn["pfcand_cov_tanLambdaz0"] = "pfcand_dlambdadz";
    map_to_FCCAn["pfcand_cov_d0c"] = "pfcand_dxyc";
    map_to_FCCAn["pfcand_cov_d0tanLambda"] = "pfcand_dxyctgtheta";
    map_to_FCCAn["pfcand_cov_phic"] = "pfcand_phic";
    map_to_FCCAn["pfcand_cov_phiz0"] = "pfcand_phidz";
    map_to_FCCAn["pfcand_cov_phitanLambda"] = "pfcand_phictgtheta";
    map_to_FCCAn["pfcand_cov_cz0"] = "pfcand_cdz";
    map_to_FCCAn["pfcand_cov_ctanLambda"] = "pfcand_cctgtheta";
    map_to_FCCAn["pfcand_d0"] = "pfcand_dxy";
    map_to_FCCAn["pfcand_z0"] = "pfcand_dz";
    map_to_FCCAn["pfcand_Sip2dVal"] = "pfcand_btagSip2dVal";
    map_to_FCCAn["pfcand_Sip2dSig"] = "pfcand_btagSip2dSig";
    map_to_FCCAn["pfcand_Sip3dVal"] = "pfcand_btagSip3dVal";
    map_to_FCCAn["pfcand_Sip3dSig"] = "pfcand_btagSip3dSig";
    map_to_FCCAn["pfcand_JetDistVal"] = "pfcand_btagJetDistVal";
    map_to_FCCAn["pfcand_JetDistSig"] = "pfcand_btagJetDistSig";
    map_to_FCCAn["pfcand_type"] = "pfcand_type";
    map_to_FCCAn["pfcand_charge"] = "pfcand_charge";
    map_to_FCCAn["pfcand_isEl"] = "pfcand_isEl";
    map_to_FCCAn["pfcand_isMu"] = "pfcand_isMu";
    map_to_FCCAn["pfcand_isGamma"] = "pfcand_isGamma";
    map_to_FCCAn["pfcand_isChargedHad"] = "pfcand_isChargedHad";
    map_to_FCCAn["pfcand_isNeutralHad"] = "pfcand_isNeutralHad";

    // Create the reverse mapping
    for (const auto& pair : map_to_FCCAn) {
        map_to_key4hep[pair.second] = pair.first;
    }
}

std::string VarMapper::mapKey4hepToFCCAn(const std::string& key4hepName) const {
    auto it = map_to_FCCAn.find(key4hepName);
    if (it != map_to_FCCAn.end()) {
        return it->second;
    }
    return ""; // Return an empty string if not found
}

std::string VarMapper::mapFCCAnToKey4hep(const std::string& FCCAnName) const {
    auto it = map_to_key4hep.find(FCCAnName);
    if (it != map_to_key4hep.end()) {
        return it->first;
    }
    return ""; // Return an empty string if not found
}
