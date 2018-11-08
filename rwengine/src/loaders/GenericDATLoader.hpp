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
typedef std::shared_ptr<VehicleInfo> VehicleInfoPtr;
typedef std::map<std::string, VehicleInfoPtr> VehicleInfoPtrs;

class GenericDATLoader {
public:
    void loadDynamicObjects(
        const std::string& name,
        std::unordered_map<std::string, DynamicObjectData>& data);

    void loadWeapons(const std::string& name,
                     std::vector<WeaponData>& weaponData);

    void loadHandling(const std::string& name, VehicleInfoPtrs& vehicleData);
};

#endif
