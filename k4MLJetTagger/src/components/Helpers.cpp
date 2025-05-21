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
#include "Helpers.h"

#include <DD4hep/DD4hepUnits.h>

double getBzAtOrigin(dd4hep::Detector* theDetector) {
  double bfield(0.0);
  if (not(theDetector->state() == dd4hep::Detector::READY)) {
    throw std::runtime_error("Detector geometry not initialised, cannot get bfield");
  }
  const double position[3] = {0, 0, 0};      // position to calculate magnetic field at (the origin in this case)
  double magneticFieldVector[3] = {0, 0, 0}; // initialise object to hold magnetic field
  theDetector->field().magneticField(position, magneticFieldVector); // get the magnetic field vector from DD4hep
  bfield = magneticFieldVector[2] / dd4hep::tesla;                   // z component at (0,0,0)
  return bfield;
}

nlohmann::json loadJsonFile(const std::string& json_path) {
  std::ifstream json_file(json_path);
  if (!json_file.is_open()) {
    std::cerr << "Failed to open JSON file: " << json_path << std::endl;
    return nlohmann::json();
  }
  nlohmann::json json_config;
  json_file >> json_config;
  return json_config;
}

const std::map<std::string, int> to_PDGflavor = {
    {"recojet_isG", 21},  // PDG value for Gluon
    {"recojet_isU", 2},   // PDG value for Up quark
    {"recojet_isS", 3},   // PDG value for Strange quark
    {"recojet_isC", 4},   // PDG value for Charm quark
    {"recojet_isB", 5},   // PDG value for Bottom quark
    {"recojet_isD", 1},   // PDG value for Down quark
    {"recojet_isTAU", 15} // PDG value for Tau lepton
};

bool check_flavors(std::vector<std::string>& flavorNames, const std::vector<std::string>& flavor_collection_names) {
  if (flavorNames.size() != flavor_collection_names.size()) {
    std::cerr << "The number of flavors in the JSON configuration file does not match the number of flavor collection "
                 "names in the python config file."
              << std::endl;
    return false;
  }

  for (size_t i = 0; i < flavorNames.size(); ++i) {
    // Extract the flavor identifier from the flavor name
    std::string flavorName = flavorNames[i];
    std::string flavorCollectionName = flavor_collection_names[i];

    // Find the flavor identifier by locating the last underscore
    size_t flavorNamePos = flavorName.find_last_of('_');
    size_t flavorCollectionPos = flavorCollectionName.find_last_of('_');

    if (flavorNamePos == std::string::npos || flavorCollectionPos == std::string::npos) {
      std::cerr << "Invalid format for flavor names or collection names at index " << i << "." << std::endl;
      return false;
    };

    std::string extractedFlavorName = flavorName.substr(flavorNamePos + 3); // +3 to remove the "is" prefix
    std::string extractedFlavorCollectionName = flavorCollectionName.substr(flavorCollectionPos + 1);

    // Compare the extracted flavors
    if (extractedFlavorName != extractedFlavorCollectionName) {
      std::cerr << "Mismatch at index " << i << ": Flavor name (" << extractedFlavorName
                << ") does not match flavor collection name (" << extractedFlavorCollectionName << ")." << std::endl;
      return false;
    };
  }

  // If all checks pass
  return true;
}

rv::RVec<rv::RVec<float>> from_Jet_to_onnx_input(Jet& jet, const rv::RVec<std::string>& input_names) {
  /**
   * Return the input variables for the ONNX model from a Jet object.
   * The input variables should have the form of {jet -> {var1 -> {constit1, constit2, ...}, var2 -> {...}, ...}}
   * @param jet: the jet object
   * @param input_names: the names of the input variables for the ONNX model.
   * @return: the input variables for the ONNX model
   */
  rv::RVec<rv::RVec<float>> constituent_vars;
  VarMapper mapper; // transform the names of the variables (ONNX (aka FCCAnalyses) convention <-> key4hep convention)
  for (auto& pfcand : jet.constituents) { // loop over all constituents
    rv::RVec<float> vars;
    // loop over all expected input observables
    for (auto& obs : input_names) {
      std::string key4hepName = mapper.mapFCCAnToKey4hep(obs); // map the variable name to the key4hep convention
      vars.push_back(pfcand.get_attribute(key4hepName));
    }
    // add the vars to the constituent_vars
    constituent_vars.push_back(vars);
  }

  // change from {constituent -> {var1, var2, ...}} to {var1 -> {constituent1, constituent2, ...}, var2 -> {...}, ...}
  rv::RVec<rv::RVec<float>> input_vars;
  for (unsigned int i = 0; i < constituent_vars[0].size(); i++) { // loop over all variables
    rv::RVec<float> var;
    for (unsigned int j = 0; j < constituent_vars.size(); j++) { // loop over all constituents
      var.push_back(constituent_vars[j][i]);
    }
    input_vars.push_back(var);
  }

  return input_vars;
};

// converstion from FCCAnalyses to key4hep and vice versa

VarMapper::VarMapper() {
  m_mapToFCCAn["pfcand_erel_log"] = "pfcand_erel_log";
  m_mapToFCCAn["pfcand_thetarel"] = "pfcand_thetarel";
  m_mapToFCCAn["pfcand_phirel"] = "pfcand_phirel";
  m_mapToFCCAn["pfcand_cov_omegaomega"] = "pfcand_dptdpt";
  m_mapToFCCAn["pfcand_cov_tanLambdatanLambda"] = "pfcand_detadeta";
  m_mapToFCCAn["pfcand_cov_phiphi"] = "pfcand_dphidphi";
  m_mapToFCCAn["pfcand_cov_d0d0"] = "pfcand_dxydxy";
  m_mapToFCCAn["pfcand_cov_z0z0"] = "pfcand_dzdz";
  m_mapToFCCAn["pfcand_cov_d0z0"] = "pfcand_dxydz";
  m_mapToFCCAn["pfcand_cov_phid0"] = "pfcand_dphidxy";
  m_mapToFCCAn["pfcand_cov_tanLambdaz0"] = "pfcand_dlambdadz";
  m_mapToFCCAn["pfcand_cov_d0omega"] = "pfcand_dxyc";
  m_mapToFCCAn["pfcand_cov_d0tanLambda"] = "pfcand_dxyctgtheta";
  m_mapToFCCAn["pfcand_cov_phiomega"] = "pfcand_phic";
  m_mapToFCCAn["pfcand_cov_phiz0"] = "pfcand_phidz";
  m_mapToFCCAn["pfcand_cov_phitanLambda"] = "pfcand_phictgtheta";
  m_mapToFCCAn["pfcand_cov_omegaz0"] = "pfcand_cdz";
  m_mapToFCCAn["pfcand_cov_omegatanLambda"] = "pfcand_cctgtheta";
  m_mapToFCCAn["pfcand_d0"] = "pfcand_dxy";
  m_mapToFCCAn["pfcand_z0"] = "pfcand_dz";
  m_mapToFCCAn["pfcand_Sip2dVal"] = "pfcand_btagSip2dVal";
  m_mapToFCCAn["pfcand_Sip2dSig"] = "pfcand_btagSip2dSig";
  m_mapToFCCAn["pfcand_Sip3dVal"] = "pfcand_btagSip3dVal";
  m_mapToFCCAn["pfcand_Sip3dSig"] = "pfcand_btagSip3dSig";
  m_mapToFCCAn["pfcand_JetDistVal"] = "pfcand_btagJetDistVal";
  m_mapToFCCAn["pfcand_JetDistSig"] = "pfcand_btagJetDistSig";
  m_mapToFCCAn["pfcand_type"] = "pfcand_type";
  m_mapToFCCAn["pfcand_charge"] = "pfcand_charge";
  m_mapToFCCAn["pfcand_isEl"] = "pfcand_isEl";
  m_mapToFCCAn["pfcand_isMu"] = "pfcand_isMu";
  m_mapToFCCAn["pfcand_isGamma"] = "pfcand_isGamma";
  m_mapToFCCAn["pfcand_isChargedHad"] = "pfcand_isChargedHad";
  m_mapToFCCAn["pfcand_isNeutralHad"] = "pfcand_isNeutralHad";
  m_mapToFCCAn["pfcand_dndx"] = "pfcand_dndx";
  m_mapToFCCAn["pfcand_tof"] = "pfcand_mtof";
  m_mapToFCCAn["pfcand_e"] = "pfcand_e";
  m_mapToFCCAn["pfcand_p"] = "pfcand_p";

  // Create the reverse mapping
  for (const auto& pair : m_mapToFCCAn) {
    m_mapToKey4hep[pair.second] = pair.first;
  }
}

std::string VarMapper::mapKey4hepToFCCAn(const std::string& key4hepName) const {
  auto it = m_mapToFCCAn.find(key4hepName);
  if (it != m_mapToFCCAn.end()) {
    return it->second;
  }
  return ""; // Return an empty string if not found
}

std::string VarMapper::mapFCCAnToKey4hep(const std::string& FCCAnName) const {
  auto it = m_mapToKey4hep.find(FCCAnName);
  if (it != m_mapToKey4hep.end()) {
    return it->second;
  }
  return ""; // Return an empty string if not found
}
