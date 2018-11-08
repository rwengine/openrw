#ifndef _RWENGINE_GENERICDATLOADER_HPP_
#define _RWENGINE_GENERICDATLOADER_HPP_

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct DynamicObjectData;
struct WeaponData;
struct VehicleInfo;

class GenericDATLoader {
public:
    void loadDynamicObjects(
        const std::string& name,
        std::unordered_map<std::string, DynamicObjectData>& data);

    void loadWeapons(const std::string& name,
                     std::vector<WeaponData>& weaponData);

    void loadHandling(
        const std::string& name,
        std::unordered_map<std::string, VehicleInfo>& vehicleData);
};

#endif
