#include "engine/SaveGame.hpp"

#include <iostream>

#include <rw/debug.hpp>
#include <rw/filesystem.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "data/ZoneData.hpp"
#include "engine/GameData.hpp"
#include "engine/GameState.hpp"
#include "engine/GameWorld.hpp"
#include "engine/SaveGameTypes.hpp"
#include "engine/SaveGameSerialization.hpp"
#include "objects/CharacterObject.hpp"
#include "objects/GameObject.hpp"
#include "objects/InstanceObject.hpp"
#include "objects/VehicleObject.hpp"
#include "script/SCMFile.hpp"
#include "script/ScriptMachine.hpp"
#include "script/ScriptTypes.hpp"

namespace {
using namespace SaveGameSerialization;

template <class T> bool SectionGeneral(T& s, GameState& state) {
    serialize(s, state.basic);

    auto b = MakeSaveBlock(s);
    if (!Magic(b, {{'S', 'C', 'R', '\0'}})) { return false; }

    return WithBlock(b, [&state](auto &b1) {
        uint32_t globalCount = 0;
        Block0ScriptData scriptData;
        serialize(b1, globalCount);

        std::vector<SCMByte> data(globalCount);
        serialize(b1, data);

        WithBlock(b1, [&scriptData](auto &b2) {
            serialize(b2, scriptData);
        });

        uint32_t numScripts{};
        serialize(b1, numScripts);
        std::vector<Block0RunningScript> scripts(numScripts);
        serialize(b1, scripts);

        // Writing is not used yet but this keeps structure above
        // from mixing with the cleanup below.
        if (T::Writing) {
            return true;
        }

        // We keep track of the game time as a float for now
        state.gameTime = state.basic.timeMS / 1000.f;

        state.scriptOnMissionFlag = reinterpret_cast<int32_t *>(
            state.script->getGlobals() +
            static_cast<size_t>(scriptData.onMissionOffset));

        auto &threads = state.script->getThreads();
        for (auto &script : scripts) {
            state.script->startThread(script.programCounter);
            SCMThread &thread = threads.back();
            // no baseAddress in III and VC
            strncpy(thread.name, script.name, sizeof(SCMThread::name) - 1);
            thread.conditionResult = script.ifFlag;
            thread.conditionCount = script.ifNumber;
            thread.stackDepth = script.stackCounter;
            for (int i = 0; i < SCM_STACK_DEPTH; ++i) {
                thread.calls[i] = script.stack[i];
            }
            /* TODO not hardcode +33 ms */
            thread.wakeCounter = script.wakeTimer - state.basic.lastTick + 33;
            for (size_t i = 0;
                 i < sizeof(Block0RunningScript::variables); ++i) {
                thread.locals[i] = script.variables[i];
            }
        }

        return true;
    });
}

template <class T> bool SectionPlayer(T& s, GameState& state) {
    auto b = MakeSaveBlock(s);

    uint32_t playerCount = 1;
    serialize(b, playerCount);

    std::vector<Block1PlayerPed> players(playerCount);
    serialize(b, players);

    if (playerCount > 0) {
        auto &ply = players[0];

#ifdef RW_DEBUG
        std::cout << "Player Health: " << ply.info.health << " ("
                  << ply.info.armour << ")" << std::endl;
        std::cout << "Player model: " << ply.modelName << std::endl;
        for (const auto &wep : ply.info.weapons) {
            std::cout << "ID " << wep.weaponId << " " << wep.inClip << " "
                      << wep.totalBullets << std::endl;
        }
#endif
        std::cout << ply.reference << std::endl;
        auto player = state.world->createPlayer(players[0].info.position);
        CharacterState &cs = player->getCurrentState();
        cs.health = players[0].info.health;
        cs.armour = players[0].info.armour;
        state.playerObject = player->getGameObjectID();
        state.maxWantedLevel = players[0].maxWantedLevel;
        for (int w = 0; w < kNrOfWeapons; ++w) {
            auto &wep = ply.info.weapons[w];
            cs.weapons[w].weaponId = wep.weaponId;
            cs.weapons[w].bulletsClip = wep.inClip;
            cs.weapons[w].bulletsTotal = wep.totalBullets;
        }
    }
    return true;
}

template <class T> bool SectionGarages(T& s, GameState& state) {
    auto b = MakeSaveBlock(s);

    Block2GarageData garageData{};
    serialize(b, garageData);

    std::vector<StructGarage> garages(garageData.garageCount);
    serialize(b, garages);

#ifdef RW_DEBUG
    std::cout << "Garages: " << garageData.garageCount << std::endl;
    std::cout << "Bombs Free: " << garageData.freeBombs << std::endl;
    std::cout << "Sprays Free: " << garageData.freeResprays << std::endl;
    std::cout << "Portland IE: " << garageData.bfImportExportPortland
              << std::endl;
    std::cout << "Shoreside IE: " << garageData.bfImportExportShoreside
              << std::endl;
    std::cout << "GA21 last shown: " << garageData.GA_21lastTime << std::endl;
    for (const auto &car : garageData.cars) {
        if (car.modelId == 0) continue;
        std::cout << " " << car.modelId << " " << uint16_t(car.colorFG) << "/"
                  << uint16_t(car.colorBG) << " " << car.immunities
                  << std::endl;
    }
#endif

    if (T::Writing) {
        return true;
    }

    state.importExportPortland = garageData.bfImportExportPortland;
    state.importExportShoreside = garageData.bfImportExportShoreside;
    state.importExportUnused = garageData.bfImportExportUnused;

    for (auto &garage : garages) {
        state.world->createGarage(glm::vec3(garage.x1, garage.y1, garage.z1),
                                  glm::vec3(garage.x2, garage.y2, garage.z2),
                                  static_cast<Garage::Type>(garage.type));
    }
    for (auto &c : garageData.cars) {
        if (c.modelId == 0) continue;
        auto &car = c;
        glm::quat rotation(
            glm::mat3(glm::cross(car.rotation, glm::vec3(0.f, 0.f, 1.f)),
                      car.rotation, glm::vec3(0.f, 0.f, 1.f)));

        VehicleObject *vehicle =
            state.world->createVehicle(car.modelId, car.position, rotation);
        vehicle->setPrimaryColour(car.colorFG);
        vehicle->setSecondaryColour(car.colorBG);
    }

    return true;
}

template <class T> bool SectionVehicles(T& s, GameState&) {
    auto b = MakeSaveBlock(s);

    BlockDword vehicleCount = 0;
    serialize(b, vehicleCount);
    BlockDword boatCount = 0;
    serialize(b, boatCount);

    std::vector<Block3Vehicle> vehicles(vehicleCount);
    serialize(b, vehicles);
    std::vector<Block3Boat> boats(boatCount);
    serialize(b, boats);

#ifdef RW_DEBUG
    for (auto &veh : vehicles) {
        std::cout << " v " << veh.modelId << " " << veh.state.position.x << " "
                  << veh.state.position.y << " " << veh.state.position.z
                  << std::endl;
    }
    for (auto &veh : boats) {
        std::cout << " b " << veh.modelId << " " << veh.state.position.x << " "
                  << veh.state.position.y << " " << veh.state.position.z
                  << std::endl;
    }
#endif

    return true;
}

template <class T> bool SectionObjects(T& s, GameState& state) {
    auto b = MakeSaveBlock(s);

    BlockDword objectCount;
    serialize(b, objectCount);

    std::vector<Block4Object> objects(objectCount);
    serialize(b, objects);

#ifdef RW_DEBUG
    std::cout << "Objects " << objectCount << "\n";
#endif
    for (auto &obj : objects) {
        GameObject *inst =
            state.world->createInstance(obj.modelId, obj.position);
        glm::vec3 right = glm::normalize(
            glm::vec3(obj.rotation[0], obj.rotation[1], obj.rotation[2]));
        glm::vec3 forward = glm::normalize(
            glm::vec3(obj.rotation[3], obj.rotation[4], obj.rotation[5]));
        glm::vec3 down = glm::normalize(
            glm::vec3(obj.rotation[6], obj.rotation[7], obj.rotation[8]));
        glm::mat3 m = glm::mat3(right, forward, -down);
        inst->setRotation(glm::normalize(static_cast<glm::quat>(m)));
#ifdef RW_DEBUG
        std::cout << "modelId " << obj.modelId << " ";
        std::cout << "position " << obj.position.x << " " << obj.position.y
                  << " " << obj.position.z << " ";
        std::cout << "right " << right.x << " " << right.y << " " << right.z
                  << " ";
        std::cout << "forward " << forward.x << " " << forward.y << " "
                  << forward.z << " ";
        std::cout << "down " << down.x << " " << down.y << " " << down.z
                  << std::endl;
#endif
    }

    return true;
}

template <class T> bool SectionPaths(T& s, GameState&) {
    auto b = MakeSaveBlock(s);

    BlockDword numPaths;
    serialize(b, numPaths);
    std::vector<uint8_t> pathData(numPaths);
    serialize(b, pathData);

    return true;
}

template <class T> bool SectionCranes(T& s, GameState&) {
    auto b = MakeSaveBlock(s);

    Block6Data craneData;
    serialize(b, craneData);

    return true;
}

template <class T> bool SectionPickups(T& s, GameState&) {
    auto b = MakeSaveBlock(s);

    Block7Data pickupData;
    serialize(b, pickupData);

    return true;
}

template <class T> bool SectionPayphones(T& s, GameState& state) {
    auto b = MakeSaveBlock(s);

    Block8Data payphoneData{};
    serialize(b, payphoneData);
    std::vector<Block8Payphone> payphones(payphoneData.numPayphones);
    serialize(b, payphones);

#ifdef RW_DEBUG
    std::cout << "Payphones: " << payphoneData.numPayphones << std::endl;
    for (const auto &payphone : payphones) {
        std::cout << " " << uint16_t(payphone.state) << " "
                  << payphone.position.x
                  << " " << payphone.position.y << " " << payphone.position.z
                  << std::endl;
    }
#endif

    if (T::Writing) {
        return true;
    }

    for (const auto &payphone : payphones) {
        state.world->createPayphone(glm::vec2(payphone.position));
    }

    return true;
}

template <class T> bool SectionRestarts(T& s, GameState&) {
    auto b = MakeSaveBlock(s);

    if (!Magic(b, {{'R', 'S', 'T', '\0'}})) { return false; }

    return WithBlock(b, [&](auto &b) {
        Block9Data restartData;
        serialize(b, restartData);

#ifdef RW_DEBUG
        std::cout << "Hospitals: " << restartData.numHospitals
                  << " police: " << restartData.numPolice << std::endl;
        for (int s = 0; s < restartData.numHospitals; ++s) {
            Block9Restart &p = restartData.hospitalRestarts[s];
            std::cout << " H " << p.position.x << " " << p.position.y << " "
                      << p.position.z << std::endl;
        }
        for (int s = 0; s < restartData.numPolice; ++s) {
            Block9Restart &p = restartData.policeRestarts[s];
            std::cout << " P " << p.position.x << " " << p.position.y << " "
                      << p.position.z << std::endl;
        }
#endif

        return true;
    });
}

template <class T> bool SectionRadar(T& s, GameState&) {
    auto b = MakeSaveBlock(s);

    if (!Magic(b, {{'R', 'D', 'R', '\0'}})) { return false; }

    return WithBlock(b, [&](auto &b) {
        Block10Data radarData;
        serialize(b, radarData);

#ifdef RW_DEBUG
        for (const auto &blip : radarData.blips) {
            if (blip.type == 0) continue;
            std::cout << blip.position.x << " " << blip.position.y << " "
                      << blip.position.z << std::endl;
        }
#endif

        return true;
    });
}

template <class T> bool SectionZone(T& s, GameState& state) {
    auto b = MakeSaveBlock(s);

    if (!Magic(b, {{'Z', 'N', 'S', '\0'}})) { return false; }

    return WithBlock(b, [&state](auto &b) {
        Block11Data zoneData;
        serialize(b, zoneData.currentZone);
        serialize(b, zoneData.currentLevel);
        serialize(b, zoneData.findIndex);
        serialize(b, zoneData.align);
        for (auto &zone : zoneData.navZones) {
            serialize(b, zone.name);
            serialize(b, zone.coordA);
            serialize(b, zone.coordB);
            serialize(b, zone.type);
            serialize(b, zone.level);
            serialize(b, zone.dayZoneInfo);
            serialize(b, zone.nightZoneInfo);
            serialize(b, zone.childZone);
            serialize(b, zone.parentZone);
            serialize(b, zone.siblingZone);
        }
        for (auto &info : zoneData.dayNightInfo) {
            serialize(b, info.density);
            serialize(b, info.unknown1);
            serialize(b, info.peddensity);
            serialize(b, info.copdensity);
            serialize(b, info.gangpeddensity);
            serialize(b, info.pedgroup);
        }
        serialize(b, zoneData.numNavZones);
        serialize(b, zoneData.numZoneInfos);
        for (auto &zone : zoneData.mapZones) {
            serialize(b, zone.name);
            serialize(b, zone.coordA);
            serialize(b, zone.coordB);
            serialize(b, zone.type);
            serialize(b, zone.level);
            serialize(b, zone.dayZoneInfo);
            serialize(b, zone.nightZoneInfo);
            serialize(b, zone.childZone);
            serialize(b, zone.parentZone);
            serialize(b, zone.siblingZone);
        }
        for (auto &audioZone : zoneData.audioZones) {
            serialize(b, audioZone);
        }
        serialize(b, zoneData.numMapZones);
        serialize(b, zoneData.numAudioZones);

#ifdef RW_DEBUG
        std::cout << "zones: " << zoneData.numNavZones << " "
                  << zoneData.numZoneInfos << " " << zoneData.numMapZones << " "
                  << zoneData.numAudioZones << std::endl;
        for (int z = 0; z < zoneData.numNavZones; ++z) {
            Block11Zone &zone = zoneData.navZones[z];
            std::cout << " " << zone.name << std::endl;
            auto &dayinfo = zoneData.dayNightInfo[zone.dayZoneInfo];
            std::cout << "  DAY " << dayinfo.density << " "
                      << dayinfo.peddensity
                      << " " << dayinfo.copdensity << " "
                      << " [";
            for (BlockDword gang : dayinfo.gangpeddensity) {
                std::cout << " " << gang;
            }
            std::cout << "] " << dayinfo.pedgroup << std::endl;
            for (BlockDword dw : dayinfo.unknown1) {
                std::cout << " " << dw;
            }
            std::cout << std::endl;

            auto &nightinfo = zoneData.dayNightInfo[zone.nightZoneInfo];
            std::cout << "  NIGHT " << nightinfo.density << " "
                      << nightinfo.peddensity << " " << nightinfo.copdensity
                      << " "
                      << " [";
            for (BlockDword gang : nightinfo.gangpeddensity) {
                std::cout << " " << gang;
            }
            std::cout << "] " << nightinfo.pedgroup << std::endl;
            for (BlockDword dw : nightinfo.unknown1) {
                std::cout << " " << dw;
            }
            std::cout << std::endl;
        }
        for (int z = 0; z < zoneData.numMapZones; ++z) {
            Block11Zone &zone = zoneData.mapZones[z];
            std::cout << " " << zone.name << std::endl;
        }
#endif

        // Clear existing zone data
        auto &gamezones = state.world->data->gamezones;
        gamezones.clear();
        for (int z = 0; z < zoneData.numNavZones; ++z) {
            Block11Zone &zone = zoneData.navZones[z];
            Block11ZoneInfo &day = zoneData.dayNightInfo[zone.dayZoneInfo];
            Block11ZoneInfo &night = zoneData.dayNightInfo[zone.nightZoneInfo];
            // @toodo restore gang density
            gamezones.emplace_back(zone.name, zone.type, zone.coordA,
                                   zone.coordB,
                                   zone.level, day.pedgroup, night.pedgroup);
        }
        // Re-build zone hierarchy
        for (ZoneData &zone : gamezones) {
            if (&zone == &gamezones[0]) {
                continue;
            }

            gamezones[0].insertZone(zone);
        }
        return true;
    });
}

template <class T> bool SectionGang(T& s, GameState& state) {
    auto b = MakeSaveBlock(s);

    if (!Magic(b, {{'G', 'N', 'G', '\0'}})) { return false; }

    return WithBlock(b, [&state](auto &b) {
        Block12Data gangData{};
        serialize(b, gangData);

#ifdef RW_DEBUG
        for (const auto &gang : gangData.gangs) {
            std::cout << " " << gang.carModelId << " " << gang.weaponPrimary
                      << " "
                      << gang.weaponSecondary << std::endl;
        }
#endif
        return true;
    });
}

template <class T> bool SectionCarGenerator(T& s, GameState& state) {
    auto b = MakeSaveBlock(s);

    if (!Magic(b, {{'C', 'G', 'N', '\0'}})) { return false; }

    return WithBlock(b, [&state](auto &b) {
        Block13Data carGeneratorData{};
        serialize(b, carGeneratorData);

        std::vector<Block13CarGenerator> carGenerators(
            carGeneratorData.generatorCount);
        serialize(b, carGenerators);

#ifdef RW_DEBUG
        std::cout << "Car generators: " << carGeneratorData.generatorCount
                  << std::endl;
        for (auto &gen : carGenerators) {
            std::cout << " " << gen.modelId
                      << " " << gen.position.x
                      << " " << gen.position.y
                      << " " << gen.position.z << "\n";
        }
#endif

        if (T::Writing) {
            return true;
        }

        for (auto g = 0u; g < carGenerators.size(); ++g) {
            const auto &gen = carGenerators[g];
            state.vehicleGenerators.emplace_back(
                g, gen.position, gen.angle, gen.modelId, gen.colourFG,
                gen.colourBG,
                gen.force, gen.alarmChance, gen.lockedChance, gen.minDelay,
                gen.maxDelay, gen.timestamp,
                101  /// @todo determine where the remainingSpawns should be
            );
        }

        return true;
    });
}

template <class T> bool SectionParticle(T& s, GameState&) {
    auto b = MakeSaveBlock(s);

    BlockDword particleCount;
    serialize(b, particleCount);
    std::vector<Block14Particle> particles(particleCount);
    serialize(b, particles);

#ifdef RW_DEBUG
    std::cout << "particles: " << particleCount << std::endl;
#endif

    return true;
}

template <class T> bool SectionAudio(T& s, GameState& state) {
    auto b = MakeSaveBlock(s);

    if (!Magic(b, {{'A', 'U', 'D', '\0'}})) { return false; }

    return WithBlock(b, [&state](auto &b) {
        BlockDword audioCount;
        serialize(b, audioCount);

        std::vector<Block15AudioObject> audioObjects(audioCount);
        serialize(b, audioObjects);

#ifdef RW_DEBUG
        std::cout << "Audio Objects: " << audioCount << std::endl;
#endif
        return true;
    });
}

template <class T> bool SectionPlayerInfo(T& s, GameState& state) {
    auto b = MakeSaveBlock(s);

    serialize(b, state.playerInfo.money);
    serialize(b, state.playerInfo.unknown1);
    serialize(b, state.playerInfo.unknown2);
    serialize(b, state.playerInfo.unknown3);
    serialize(b, state.playerInfo.unknown4);
    serialize(b, state.playerInfo.displayedMoney);
    serialize(b, state.playerInfo.hiddenPackagesCollected);
    serialize(b, state.playerInfo.hiddenPackageCount);
    serialize(b, state.playerInfo.neverTired);
    serialize(b, state.playerInfo.fastReload);
    serialize(b, state.playerInfo.thaneOfLibertyCity);
    serialize(b, state.playerInfo.singlePayerHealthcare);
    serialize(b, state.playerInfo.unknown5);

#ifdef RW_DEBUG
    std::cout << "Player money: " << state.playerInfo.money << " ("
              << state.playerInfo.displayedMoney << ")" << std::endl;
#endif

    return true;
}

template <class T> bool SectionStats(T& s, GameState& state) {
    auto b = MakeSaveBlock(s);

    serialize(b, state.gameStats.playerKills);
    serialize(b, state.gameStats.otherKills);
    serialize(b, state.gameStats.carsExploded);
    serialize(b, state.gameStats.shotsHit);
    serialize(b, state.gameStats.pedTypesKilled);
    serialize(b, state.gameStats.helicoptersDestroyed);
    serialize(b, state.gameStats.playerProgress);
    serialize(b, state.gameStats.explosiveKgsUsed);
    serialize(b, state.gameStats.bulletsFired);
    serialize(b, state.gameStats.bulletsHit);
    serialize(b, state.gameStats.carsCrushed);
    serialize(b, state.gameStats.headshots);
    serialize(b, state.gameStats.timesBusted);
    serialize(b, state.gameStats.timesHospital);
    serialize(b, state.gameStats.daysPassed);
    serialize(b, state.gameStats.mmRainfall);
    serialize(b, state.gameStats.insaneJumpMaxDistance);
    serialize(b, state.gameStats.insaneJumpMaxHeight);
    serialize(b, state.gameStats.insaneJumpMaxFlips);
    serialize(b, state.gameStats.insaneJumpMaxRotation);
    serialize(b, state.gameStats.bestStunt);
    serialize(b, state.gameStats.uniqueStuntsFound);
    serialize(b, state.gameStats.uniqueStuntsTotal);
    serialize(b, state.gameStats.missionAttempts);
    serialize(b, state.gameStats.missionsPassed);
    serialize(b, state.gameStats.passengersDroppedOff);
    serialize(b, state.gameStats.taxiRevenue);
    serialize(b, state.gameStats.portlandPassed);
    serialize(b, state.gameStats.stauntonPassed);
    serialize(b, state.gameStats.shoresidePassed);
    serialize(b, state.gameStats.bestTurismoTime);
    serialize(b, state.gameStats.distanceWalked);
    serialize(b, state.gameStats.distanceDriven);
    serialize(b, state.gameStats.patriotPlaygroundTime);
    serialize(b, state.gameStats.aRideInTheParkTime);
    serialize(b, state.gameStats.grippedTime);
    serialize(b, state.gameStats.multistoryMayhemTime);
    serialize(b, state.gameStats.peopleSaved);
    serialize(b, state.gameStats.criminalsKilled);
    serialize(b, state.gameStats.highestParamedicLevel);
    serialize(b, state.gameStats.firesExtinguished);
    serialize(b, state.gameStats.longestDodoFlight);
    serialize(b, state.gameStats.bombDefusalTime);
    serialize(b, state.gameStats.rampagesPassed);
    serialize(b, state.gameStats.totalRampages);
    serialize(b, state.gameStats.totalMissions);
    serialize(b, state.gameStats.fastestTime);
    serialize(b, state.gameStats.highestScore);
    serialize(b, state.gameStats.peopleKilledSinceCheckpoint);
    serialize(b, state.gameStats.peopleKilledSinceLastBustedOrWasted);
    serialize(b, state.gameStats.lastMissionGXT);

#ifdef RW_DEBUG
    std::cout << "Player kills: " << state.gameStats.playerKills << std::endl;
    std::cout << "longest flight " << state.gameStats.longestDodoFlight
              << std::endl;
#endif
    return true;
}

template <class T> bool SectionStreaming(T& s, GameState&) {
    auto b = MakeSaveBlock(s);

    Block18Data streamingData{};
    serialize(b, streamingData);

    return true;
}

template <class T> bool SectionPedTypes(T& s, GameState&) {
    auto b = MakeSaveBlock(s);

    if (!Magic(b, {{'P', 'T', 'P', '\0'}})) { return false; }

    Block19Data pedTypeData{};
    serialize(b, pedTypeData);

#ifdef RW_DEBUG
    for (const auto &type : pedTypeData.types) {
        printf("%08x: %f %f %f %f %f threat %08x avoid %08x\n", type.bitstring_,
               static_cast<double>(type.unknown2),
               static_cast<double>(type.unknown3),
               static_cast<double>(type.unknown4),
               static_cast<double>(type.fleedistance),
               static_cast<double>(type.headingchangerate),
               type.threatflags_,
               type.avoidflags_);
    }
#endif

    return true;
}

template <class T> bool SerializeGame(T& s, GameState& state) {
    static_assert(!T::Writing);
    constexpr bool(*handlers[])(SaveBlock<T>&,GameState&)  {
            &SectionGeneral,
            &SectionPlayer,
            &SectionGarages,
            &SectionVehicles,
            &SectionObjects,
            &SectionPaths,
            &SectionCranes,
            &SectionPickups,
            &SectionPayphones,
            &SectionRestarts,
            &SectionRadar, // 10 Radar
            &SectionZone, // 11 Zone
            &SectionGang, // 12 Gangs
            &SectionCarGenerator, // 13 Car Generators
            &SectionParticle, // 14 Particles
            &SectionAudio, // 15 Audio
            &SectionPlayerInfo, // 16 Player Info
            &SectionStats, // 17 Stats
            &SectionStreaming, // 18 Streaming
            &SectionPedTypes, // 19 Ped Types
    };

    return std::all_of(std::begin(handlers), std::end(handlers),
        [&](auto& handler) {
            auto block = MakeSaveBlock(s);
            return handler(block, state);
        });
}

}

void SaveGame::writeGame(GameState& state, const std::string& file) {
    RW_UNUSED(state);
    RW_UNUSED(file);
    RW_UNIMPLEMENTED("Saving the game is not implemented yet.");
}

bool SaveGame::loadGame(GameState& state, const std::string& file) {
    if (std::ifstream saveFile {file, std::ios_base::binary}; saveFile) {
        SaveGameSerialization::Reader reader {saveFile};
        return SerializeGame(reader, state);
    }
    std::cerr << "Failed to open save file " << file << "\n";
    return false;
}

bool SaveGame::getSaveInfo(const std::string& file, BasicState& basicState) {
    if (std::ifstream saveFile {file, std::ios_base::binary}; saveFile) {
        SaveGameSerialization::Reader reader {saveFile};
        return SaveGameSerialization::WithBlock(reader, [&](auto& b) {
            return serialize(b, basicState);
        });
    }
    std::cerr << "Failed to open save file " << file << "\n";
    return false;
}

#ifdef RW_WINDOWS
char* readUserPath() {
    LONG retval;
    const char* lpSubKey = "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders";
    const char* lpValueName = "Personal";
    DWORD lpType = REG_SZ;
    HKEY phkResult;
    static char lpData[1000];
    DWORD lpcbData = sizeof(lpData);

    retval = RegOpenKeyEx(HKEY_CURRENT_USER, lpSubKey, 0, KEY_READ, &phkResult);
    if (ERROR_SUCCESS != retval) { return nullptr; }

    retval = RegQueryValueEx(phkResult, lpValueName, NULL, &lpType, (LPBYTE) lpData, &lpcbData);
    if (ERROR_SUCCESS != retval) { return nullptr; }

    retval = RegCloseKey(phkResult);
    if (ERROR_SUCCESS != retval) { return nullptr; }

    return lpData;
}
#endif

std::vector<SaveGameInfo> SaveGame::getAllSaveGameInfo() {
#ifdef RW_WINDOWS
    auto homedir = readUserPath(); // already includes MyDocuments/Documents
#else
    auto homedir = getenv("HOME");
#endif
    if (homedir == nullptr) {
        std::cerr << "Unable to determine home directory" << std::endl;
        return {};
    }
    const char gameDir[] = "GTA3 User Files";

    rwfs::path gamePath(homedir);
    gamePath /= gameDir;

    if (!rwfs::exists(gamePath) || !rwfs::is_directory(gamePath)) return {};

    std::vector<SaveGameInfo> infos;
    for (const rwfs::path& save_path : rwfs::directory_iterator(gamePath)) {
        if (save_path.extension() == ".b") {
            infos.emplace_back(
                SaveGameInfo{save_path.string(), false, BasicState()});
            infos.back().valid =
                getSaveInfo(infos.back().savePath, infos.back().basicState);
        }
    }

    return infos;
}
