#ifndef _GENERICDATLOADER_HPP_
#define _GENERICDATLOADER_HPP_

#include <map>
#include <memory>
#include <vector>

struct DynamicObjectData;
typedef std::shared_ptr<DynamicObjectData> DynamicObjectDataPtr;
typedef std::map<std::string,DynamicObjectDataPtr> DynamicObjectDataPtrs;

struct WeaponData;
typedef std::shared_ptr<WeaponData> WeaponDataPtr;
typedef std::vector<WeaponDataPtr> WeaponDataPtrs;

struct VehicleInfo;
typedef std::shared_ptr<VehicleInfo> VehicleInfoPtr;
typedef std::map<std::string, VehicleInfoPtr> VehicleInfoPtrs;

class GenericDATLoader
{
public:

	void loadDynamicObjects(const std::string& name, DynamicObjectDataPtrs& data);

	void loadWeapons(const std::string& name, WeaponDataPtrs& weaponData);

	void loadHandling(const std::string& name, VehicleInfoPtrs& vehicleData);
};

#endif
