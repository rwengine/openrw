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
