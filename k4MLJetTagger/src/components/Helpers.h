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
#ifndef HELPERS_H
#define HELPERS_H

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp> // Include a JSON parsing library
#include <string>
#include <unordered_map>

#include "ROOT/RVec.hxx"
#include "Structs.h"

#include <DD4hep/Detector.h> // for Bfield

namespace rv = ROOT::VecOps;

/**
 * Get the magnetic field at the origin by using the detector geometry.
 * Adapted from:
 * https://github.com/iLCSoft/MarlinUtil/blob/18dd0588a76902551cd06cc9499099f662b48c5f/source/src/GeometryUtil.cc#L5-L21
 * @return: the magnetic field at the origin
 */
double getBzAtOrigin(dd4hep::Detector* theDetector);

/**
 * Return the input variables for the ONNX model from a Jet object.
 * The input variables should have the form of {jet -> {var1 -> {constit1, constit2, ...}, var2 -> {...}, ...}}
 * @param jet: the jet object
 * @param input_names: the names of the input variables for the ONNX model.
 * @return: the input variables for the ONNX model
 */
rv::RVec<rv::RVec<float>> from_Jet_to_onnx_input(Jet& jet, const rv::RVec<std::string>& input_names);

/**
 * Load a JSON file from a given path.
 * @param json_path: the path to the JSON file
 * @return: the JSON object
 */
nlohmann::json loadJsonFile(const std::string& json_path);

/**
 * Map the flavor names from weaver convention to the corresponding PDG values.
 */
extern const std::map<std::string, int> to_PDGflavor;

/** Check if the flavor names from the JSON configuration file match the flavor collection names in the python config
 * file
 * @param flavorNames: the flavor names from the JSON configuration file
 * @param flavor_collection_names: the flavor collection names from the python config file
 * @return: true if the flavor names match the flavor collection names, false otherwise
 */
bool check_flavors(std::vector<std::string>& flavorNames, const std::vector<std::string>& flavor_collection_names);

/**
 * @class VarMapper
 * @brief A utility class for mapping variable names between FCCAnalyses and Key4HEP conventions.
 *
 * This class provides methods to map variable names used in FCCAnalyses to those in Key4HEP
 * and vice versa. It maintains two internal mappings for bidirectional conversion.
 *
 * Example:
 *   VarMapper mapper;
 *   std::string FCCAnName = mapper.mapKey4hepToFCCAn("key4hep_variable");
 *   std::string key4hepName = mapper.mapFCCAnToKey4hep("FCCAn_variable");
 */
class VarMapper {
public:
  /**
   * @brief Default constructor.
   *
   * Initializes the variable mappings between FCCAnalyses and Key4HEP.
   */
  VarMapper();

  /**
   * @brief Maps a Key4HEP variable name to an FCCAnalyses variable name.
   *
   * @param key4hepName The variable name in Key4HEP format.
   * @return The corresponding variable name in FCCAnalyses format.
   */
  std::string mapKey4hepToFCCAn(const std::string& key4hepName) const;

  /**
   * @brief Maps an FCCAnalyses variable name to a Key4HEP variable name.
   *
   * @param FCCAnName The variable name in FCCAnalyses format.
   * @return The corresponding variable name in Key4HEP format.
   */
  std::string mapFCCAnToKey4hep(const std::string& FCCAnName) const;

private:
  /**
   * @brief Mapping from Key4HEP variable names to FCCAnalyses variable names.
   */
  std::unordered_map<std::string, std::string> map_to_FCCAn;

  /**
   * @brief Mapping from FCCAnalyses variable names to Key4HEP variable names.
   */
  std::unordered_map<std::string, std::string> map_to_key4hep;
};

#endif // HELPERS_H