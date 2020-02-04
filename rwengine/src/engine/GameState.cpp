#include <objects/CharacterObject.hpp>

#include "engine/GameState.hpp"

int GameState::addRadarBlip(BlipData& blip) {
    int l = 0;
    for (const auto& [nr, blipData] : radarBlips) {
        if ((nr) != l) {
            l = nr - 1;
        } else {
            l++;
        }
    }

    blip.id = l;
    radarBlips.insert({l, blip});

    return l;
}

void GameState::removeBlip(int blip) {
    auto it = radarBlips.find(blip);
    if (it != radarBlips.end()) {
        radarBlips.erase(it);
    }
}

void GameState::addHospitalRestart(const glm::vec4 location) {
    hospitalRestarts.push_back(location);
}

void GameState::addPoliceRestart(const glm::vec4 location) {
    policeRestarts.push_back(location);
}

void GameState::overrideRestart(const glm::vec4 location) {
    overrideNextRestart = true;
    nextRestartLocation = location;
}

void GameState::cancelRestartOverride() {
    overrideNextRestart = false;
}

const glm::vec4 GameState::getClosestRestart(
    RestartType type, const glm::vec3 playerPosition) const {
    float closest = 10000.f;
    glm::vec4 result;

    ZoneData* playerZone = world->data->findZoneAt(playerPosition);

    const std::vector<glm::vec4>* iter = nullptr;
    int islandOverride = 0;

    if (type == Hospital) {
        iter = &hospitalRestarts;
        islandOverride = hospitalIslandOverride;
    } else if (type == Police) {
        iter = &policeRestarts;
        islandOverride = policeIslandOverride;
    }

    for (auto& location : *iter) {
        glm::vec3 location3d(location);

        ZoneData* restartZone = world->data->findZoneAt(location3d);

        if ((playerZone->island == restartZone->island &&
             islandOverride != 0) ||
            islandOverride == 0) {
            if (glm::distance(location3d, playerPosition) < closest) {
                result = location;
                closest = glm::distance(location3d, playerPosition);
            }
        }
    }

    return result;
}

void GameState::fade(float time, bool f) {
    fadeTime = time;
    fadeIn = f;
    fadeStart = world->getGameTime();
}

bool GameState::isFading() const {
    return world->getGameTime() <= fadeStart + fadeTime;
}

void GameState::setFadeColour(glm::i32vec3 colour) {
    fadeColour = colour;
}

void GameState::showHelpMessage(const GameStringKey& id) {
    text.addText<ScreenTextType::Help>(
            ScreenTextEntry::makeHelp(id, world->data->texts.text(id)));
}

Rampage::Status Rampage::getStatus() const {
    return status;
}

void Rampage::init(const std::string& gxtEntry, const int32_t weaponID,
                   const int32_t time, const int32_t kills,
                   const std::array<int32_t, 4>& modelsToKill,
                   const bool headshot) {
    // TODO: Document why
    if (gxtEntry != "PAGE_00") {
        special = true;
    }
    status = Ongoing;
    weapon = weaponID;
    endTime = state->world->getGameTime() + static_cast<float>(time) / 1000.f;
    killsRequired = kills;
    modelIDsToKill = modelsToKill;
    headshotOnly = headshot;
}

void Rampage::tick(float dt) {
    RW_UNUSED(dt);
    if (getStatus() != Ongoing) {
        return;
    }

    if (endTime <= state->world->getGameTime()) {
        status = Failed;
    }

    if (killsRequired <= 0) {
        if (!special) {
            state->gameStats.rampagesPassed++;

            // First rampage rewards 5,000$, second 10,000$ etc.
            state->playerInfo.money += state->gameStats.rampagesPassed * 5000;
        }

        status = Passed;
    }

}

float Rampage::getRemainingTime() const {
    // TODO: Gets called before `state->world` is populated, fix this properly.
    if (!state->world) {
        return 0;
    }

    return endTime - state->world->getGameTime();
}

uint32_t Rampage::getKillsForThisModel(ModelID model) {
    const auto& search = modelIDsKilled.find(model);
    if (search != modelIDsKilled.end()) {
        return search->second;
    }
    return 0;
}

void Rampage::clearKills() {
    modelIDsKilled.clear();
}

void Rampage::onCharacterDie(CharacterObject* victim, GameObject* killer) {
    if (getStatus() != Ongoing) {
        return;
    }

    if (killer) {
        if (!static_cast<CharacterObject*>(killer)->isPlayer()) {
            return;
        }
    }

    const auto modelID = victim->getModelInfo<BaseModelInfo>()->id();
    const auto isRightModel = std::find(std::begin(modelIDsToKill),
                                        std::end(modelIDsToKill), modelID);

    if (isRightModel != std::end(modelIDsToKill)) {
        modelIDsKilled[modelID]++;
        killsRequired--;
    }
}
