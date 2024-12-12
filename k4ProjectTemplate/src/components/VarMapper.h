#ifndef VARMAPPER_H
#define VARMAPPER_H

#include <unordered_map>
#include <string>

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

#endif // VARMAPPER_H
