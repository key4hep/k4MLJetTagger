#ifndef VARMAPPER_H
#define VARMAPPER_H

#include <unordered_map>
#include <string>

class VarMapper {
public:
    VarMapper();
    std::string mapKey4hepToFCCAn(const std::string& key4hepName) const;
    std::string mapFCCAnToKey4hep(const std::string& FCCAnName) const;

private:
    std::unordered_map<std::string, std::string> map_to_FCCAn;
    std::unordered_map<std::string, std::string> map_to_key4hep;
};

#endif // VarMapper_H
