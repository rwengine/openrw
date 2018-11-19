#include <loaders/GenericDATLoader.hpp>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iterator>
#include <sstream>

#include <rw/debug.hpp>

#include <data/ModelData.hpp>
#include <data/WeaponData.hpp>
#include <objects/VehicleInfo.hpp>

void GenericDATLoader::loadDynamicObjects(
    const std::string& name,
    std::unordered_map<std::string, DynamicObjectData>& data) {
    std::ifstream dfile(name.c_str());

    if (dfile.is_open()) {
        std::string lineBuff;

        while (std::getline(dfile, lineBuff)) {
            if (lineBuff.at(0) == ';') {
                continue;
            }
            if (lineBuff.at(0) == '*') {
                continue;
            }
            std::stringstream ss(lineBuff);

            DynamicObjectData dyndata{};
            std::string modelName;

            ss >> modelName;
            auto cpos = modelName.find(',');
            if (cpos != modelName.npos) {
                modelName.erase(cpos);
            }
            ss >> dyndata.mass;
            if (ss.peek() == ',') {
                ss.ignore(1);
            }
            ss >> dyndata.turnMass;
            if (ss.peek() == ',') {
                ss.ignore(1);
            }
            ss >> dyndata.airRes;
            if (ss.peek() == ',') {
                ss.ignore(1);
            }
            ss >> dyndata.elasticity;
            if (ss.peek() == ',') {
                ss.ignore(1);
            }
            ss >> dyndata.buoyancy;
            if (ss.peek() == ',') {
                ss.ignore(1);
            }
            ss >> dyndata.uprootForce;
            if (ss.peek() == ',') {
                ss.ignore(1);
            }
            ss >> dyndata.collDamageMulti;
            if (ss.peek() == ',') {
                ss.ignore(1);
            }
            int tmp;
            ss >> tmp;
            dyndata.collDamageEffect = tmp;
            if (ss.peek() == ',') {
                ss.ignore(1);
            }
            ss >> tmp;
            dyndata.collResponseFlags = tmp;
            if (ss.peek() == ',') {
                ss.ignore(1);
            }
            ss >> dyndata.cameraAvoid;

            RW_CHECK(ss.eof() || ss.good(), "Loading dynamicsObject data file " << name << " failed");

            data.emplace(modelName, std::move(dyndata));
        }
    }
}

void GenericDATLoader::loadWeapons(const std::string& name,
                                   std::vector<WeaponData>& weaponData) {
    std::ifstream dfile(name.c_str());

    if (dfile.is_open()) {
        std::string linebuffer;
        int slotNum = 0;

        while (std::getline(dfile, linebuffer)) {
            if (linebuffer[0] == '#') {
                continue;
            }
            std::stringstream ss(linebuffer);

            WeaponData data{};
            ss >> data.name;
            if (data.name == "ENDWEAPONDATA") {
                continue;
            }

            // Skip lines with blank names (probably an empty line).
            if (std::find_if(data.name.begin(), data.name.end(), ::isalnum) ==
                std::end(data.name)) {
                continue;
            }

            std::transform(data.name.begin(), data.name.end(),
                           data.name.begin(), ::tolower);

            std::string firetype;
            ss >> firetype;
            if (firetype == "MELEE") {
                data.fireType = WeaponData::MELEE;
            } else if (firetype == "INSTANT_HIT") {
                data.fireType = WeaponData::INSTANT_HIT;
            } else if (firetype == "PROJECTILE") {
                data.fireType = WeaponData::PROJECTILE;
            }

            ss >> data.hitRange;
            ss >> data.fireRate;
            ss >> data.reloadMS;
            ss >> data.clipSize;
            ss >> data.damage;
            ss >> data.speed;
            ss >> data.meleeRadius;
            ss >> data.lifeSpan;
            ss >> data.spread;
            ss >> data.fireOffset.x;
            ss >> data.fireOffset.y;
            ss >> data.fireOffset.z;
            ss >> data.animation1;
            std::transform(data.animation1.begin(), data.animation1.end(),
                           data.animation1.begin(), ::tolower);
            ss >> data.animation2;
            std::transform(data.animation2.begin(), data.animation2.end(),
                           data.animation2.begin(), ::tolower);
            ss >> data.animLoopStart;
            ss >> data.animLoopEnd;
            ss >> data.animFirePoint;
            ss >> data.animCrouchFirePoint;
            ss >> data.modelID;
            ss >> data.flags;

            RW_CHECK(ss.eof() || ss.good(), "Loading weapon data file " << name << " failed");

            data.inventorySlot = slotNum++;

            weaponData.push_back(std::move(data));
        }
    }
}

void GenericDATLoader::loadHandling(
    const std::string& name,
    std::unordered_map<std::string, VehicleInfo>& vehicleData) {
    std::ifstream hndFile(name.c_str());

    if (hndFile.is_open()) {
        std::string lineBuff;

        while (std::getline(hndFile, lineBuff)) {
            if (lineBuff.at(0) == ';') continue;
            std::stringstream ss(lineBuff);

            VehicleHandlingInfo info;
            std::string ID;

            ss >> ID;
            ss >> info.mass;
            ss >> info.dimensions.x;
            ss >> info.dimensions.y;
            ss >> info.dimensions.z;
            ss >> info.centerOfMass.x;
            ss >> info.centerOfMass.y;
            ss >> info.centerOfMass.z;
            ss >> info.percentSubmerged;
            ss >> info.tractionMulti;
            ss >> info.tractionLoss;
            ss >> info.tractionBias;
            ss >> info.numGears;
            ss >> info.maxVelocity;
            ss >> info.acceleration;
            char dt, et;
            ss >> dt;
            ss >> et;
            info.driveType = static_cast<VehicleHandlingInfo::DriveType>(dt);
            info.engineType = static_cast<VehicleHandlingInfo::EngineType>(et);
            ss >> info.brakeDeceleration;
            ss >> info.brakeBias;
            ss >> info.ABS;
            ss >> info.steeringLock;
            ss >> info.suspensionForce;
            ss >> info.suspensionDamping;
            ss >> info.seatOffset;
            ss >> info.damageMulti;
            ss >> info.value;
            ss >> info.suspensionUpperLimit;
            ss >> info.suspensionLowerLimit;
            ss >> info.suspensionBias;
            ss >> std::hex >> info.flags;

            RW_CHECK(ss.eof() || ss.good(), "Loading handling data file " << name << " failed");

            auto mit = vehicleData.find(ID);

            if (mit == vehicleData.end()) {
                vehicleData.emplace(
                    std::move(ID),
                    VehicleInfo{std::move(info), std::vector<WheelInfo>{},
                                VehicleInfo::Seats{}});
            } else {
                mit->second.handling = info;
            }
        }
    }
}
