#include "DebugState.hpp"
#include <ai/PlayerController.hpp>
#include <data/WeaponData.hpp>
#include <engine/GameState.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <objects/CharacterObject.hpp>
#include <objects/InstanceObject.hpp>
#include <objects/VehicleObject.hpp>
#include <script/SCMFile.hpp>
#include <sstream>
#include "RWGame.hpp"

constexpr float kDebugEntryHeight = 14.f;
constexpr float kDebugEntryHeightMissions = 12.f;
constexpr int kDebugFont = 2;
const glm::vec2 kDebugMenuOffset = glm::vec2(10.f, 50.f);

static void jumpCharacter(RWGame* game, CharacterObject* player,
                          const glm::vec3& target, bool ground = true) {
    glm::vec3 newPosition = target;
    if (ground) {
        newPosition = game->getWorld()->getGroundAtPosition(newPosition) +
                      glm::vec3(0.f, 0.f, 1.f);
    }
    if (player) {
        if (player->getCurrentVehicle()) {
            player->getCurrentVehicle()->setPosition(newPosition);
        } else {
            player->setPosition(newPosition);
        }
    }
}

std::shared_ptr<Menu> DebugState::createDebugMenu() {
    CharacterObject* player = nullptr;
    if (game->getWorld()->getPlayer()) {
        player = game->getWorld()->getPlayer()->getCharacter();
    }

    auto menu = Menu::create(
        {{"Jump to Debug Camera",
          [=] {
              jumpCharacter(game, player,
                            _debugCam.position +
                                _debugCam.rotation * glm::vec3(3.f, 0.f, 0.f),
                            false);
          }},
         {"-Map", [=] { this->enterMenu(createMapMenu()); }},
         {"-Vehicles", [=] { this->enterMenu(createVehicleMenu()); }},
         {"-AI", [=] { this->enterMenu(createAIMenu()); }},
         {"-Weapons", [=] { this->enterMenu(createWeaponMenu()); }},
         {"-Weather", [=] { this->enterMenu(createWeatherMenu()); }},
         {"-Missions", [=] { this->enterMenu(createMissionsMenu()); }},
         {"Set Super Jump", [=] { player->setJumpSpeed(20.f); }},
         {"Set Normal Jump",
          [=] { player->setJumpSpeed(CharacterObject::DefaultJumpSpeed); }},
         {"Full Health", [=] { player->getCurrentState().health = 100.f; }},
         {"Full Armour", [=] { player->getCurrentState().armour = 100.f; }},
         {"Cull Here",
          [=] { game->getRenderer().setCullOverride(true, _debugCam); }}},
        kDebugFont, kDebugEntryHeight);

    menu->offset = kDebugMenuOffset;

    // Optional block if the player is in a vehicle
    auto cv = player->getCurrentVehicle();
    if (cv) {
        menu->lambda("Flip vehicle", [=] {
            cv->setRotation(cv->getRotation() *
                            glm::quat(glm::vec3(0.f, glm::pi<float>(), 0.f)));
        });
    }

    return menu;
}

std::shared_ptr<Menu> DebugState::createMapMenu() {
    CharacterObject* player = nullptr;
    if (game->getWorld()->getPlayer()) {
        player = game->getWorld()->getPlayer()->getCharacter();
    }

    auto menu = Menu::create(
        {{"Back", [=] { enterMenu(createDebugMenu()); }},
         {"Jump to Docks",
          [=] {
              jumpCharacter(game, player, glm::vec3(1390.f, -837.f, 100.f));
          }},
         {"Jump to Garage",
          [=] { jumpCharacter(game, player, glm::vec3(270.f, -605.f, 40.f)); }},
         {"Jump to Airport",
          [=] {
              jumpCharacter(game, player, glm::vec3(-950.f, -980.f, 12.f));
          }},
         {"Jump to Hideout",
          [=] {
              jumpCharacter(game, player, glm::vec3(875.0, -309.0, 100.0));
          }},
         {"Jump to Luigi's",
          [=] {
              jumpCharacter(game, player, glm::vec3(902.75, -425.56, 100.0));
          }},
         {"Jump to Hospital",
          [=] {
              jumpCharacter(game, player, glm::vec3(1123.77, -569.15, 100.0));
          }},
         {"Unsolid garage doors",
          [=] {
              std::vector<std::string> garageDoorModels{
                  "8ballsuburbandoor",  "amcogaragedoor",
                  "bankjobdoor",        "bombdoor",
                  "crushercrush",       "crushertop",
                  "door2_garage",       "door3_garage",
                  "door4_garage",       "door_bombshop",
                  "door_col_compnd_01", "door_col_compnd_02",
                  "door_col_compnd_03", "door_col_compnd_04",
                  "door_col_compnd_05", "door_jmsgrage",
                  "door_sfehousegrge",  "double_garage_dr",
                  "impex_door",         "impexpsubgrgdoor",
                  "ind_plyrwoor",       "ind_slidedoor",
                  "jamesgrge_kb",       "leveldoor2",
                  "oddjgaragdoor",      "plysve_gragedoor",
                  "SalvGarage",         "shedgaragedoor",
                  "Sub_sprayshopdoor",  "towergaragedoor1",
                  "towergaragedoor2",   "towergaragedoor3",
                  "vheistlocdoor"};

              auto gw = game->getWorld();
              for (auto& i : gw->instancePool.objects) {
                  auto obj = static_cast<InstanceObject*>(i.second);
                  if (std::find(garageDoorModels.begin(),
                                garageDoorModels.end(),
                                obj->getModelInfo<BaseModelInfo>()->name) !=
                      garageDoorModels.end()) {
                      obj->setSolid(false);
                  }
              }
          }}},
        kDebugFont, kDebugEntryHeight);

    menu->offset = kDebugMenuOffset;
    return menu;
}

std::shared_ptr<Menu> DebugState::createVehicleMenu() {
    auto menu = Menu::create({{"Back", [=] { enterMenu(createDebugMenu()); }}},
                             kDebugFont, kDebugEntryHeight);

    const std::map<std::string, int> kVehicleTypes = {
        {"Landstalker", 90}, {"Taxi", 110},      {"Firetruck", 97},
        {"Police", 116},     {"Ambulance", 106}, {"Bobcat", 112},
        {"Banshee", 119},    {"Rhino", 122},     {"Barracks", 123},
        {"Rumpo", 130},      {"Columbian", 138}, {"Dodo", 126},
        {"Speeder", 142},    {"Yakuza", 136},
    };

    for (auto& e : kVehicleTypes) {
        menu->lambda(e.first, [=] { spawnVehicle(e.second); });
    }

    menu->offset = kDebugMenuOffset;
    return menu;
}

std::shared_ptr<Menu> DebugState::createAIMenu() {
    auto menu =
        Menu::create({{"Back", [=] { this->enterMenu(createDebugMenu()); }}},
                     kDebugFont, kDebugEntryHeight);

    const std::map<std::string, int> kPedTypes = {
        {"Triad", 12}, {"Cop", 1},     {"SWAT", 2},
        {"FBI", 3},    {"Fireman", 6}, {"Construction", 74},
    };

    for (auto& e : kPedTypes) {
        menu->lambda(e.first + " Follower", [=] { spawnFollower(e.second); });
    }

    menu->lambda("Kill All Peds", [=] {
        for (auto& p : game->getWorld()->pedestrianPool.objects) {
            if (p.second->getLifetime() == GameObject::PlayerLifetime) {
                continue;
            }
            p.second->takeDamage({p.second->getPosition(),
                                  p.second->getPosition(), 100.f,
                                  GameObject::DamageInfo::Explosion, 0.f});
        }
    });

    menu->offset = kDebugMenuOffset;
    return menu;
}

std::shared_ptr<Menu> DebugState::createWeaponMenu() {
    auto menu =
        Menu::create({{"Back", [=] { this->enterMenu(createDebugMenu()); }}},
                     kDebugFont, kDebugEntryHeight);

    for (int i = 1; i < kMaxInventorySlots; ++i) {
        auto& name = getWorld()->data->weaponData[i]->name;
        menu->lambda(name, [=] { giveItem(i); });
    }

    menu->offset = kDebugMenuOffset;
    return menu;
}

std::shared_ptr<Menu> DebugState::createWeatherMenu() {
    auto menu =
        Menu::create({{"Back", [=] { this->enterMenu(createDebugMenu()); }}},
                     kDebugFont, kDebugEntryHeight);

    const std::array<std::string, 4> w{"Sunny", "Cloudy", "Rainy", "Foggy"};

    for (std::size_t i = 0; i < w.size(); ++i) {
        menu->lambda(w[i],
                     [=] { game->getWorld()->state->basic.nextWeather = i; });
    }

    menu->offset = kDebugMenuOffset;
    return menu;
}

std::shared_ptr<Menu> DebugState::createMissionsMenu() {
    auto menu =
        Menu::create({{"Back", [=] { this->enterMenu(createDebugMenu()); }}},
                     kDebugFont, kDebugEntryHeightMissions);

    const std::array<std::string, 80> w{
        "Intro Movie",
        "Hospital Info Scene",
        "Police Station Info Scene",
        "RC Diablo Destruction",
        "RC Mafia Massacre",
        "RC Rumpo Rampage",
        "RC Casino Calamity",
        "Patriot Playground",
        "A Ride In The Park",
        "Gripped!",
        "Multistorey Mayhem",
        "Paramedic",
        "Firefighter",
        "Vigilante",
        "Taxi Driver",
        "The Crook",
        "The Thieves",
        "The Wife",
        "Her Lover",
        "Give Me Liberty and Luigi's Girls",
        "Don't Spank My Bitch Up",
        "Drive Misty For Me",
        "Pump-Action Pimp",
        "The Fuzz Ball",
        "Mike Lips Last Lunch",
        "Farewell 'Chunky' Lee Chong",
        "Van Heist",
        "Cipriani's Chauffeur",
        "Dead Skunk In The Trunk",
        "The Getaway",
        "Taking Out The Laundry",
        "The Pick-Up",
        "Salvatore's Called A Meeting",
        "Triads And Tribulations",
        "Blow Fish",
        "Chaperone",
        "Cutting The Grass",
        "Bomb Da Base: Act I",
        "Bomb Da Base: Act II",
        "Last Requests",
        "Turismo",
        "I Scream, You Scream",
        "Trial By Fire",
        "Big'N'Veiny",
        "Sayonara Salvatore",
        "Under Surveillance",
        "Paparazzi Purge",
        "Payday For Ray",
        "Two-Faced Tanner",
        "Kanbu Bust-Out",
        "Grand Theft Auto",
        "Deal Steal",
        "Shima",
        "Smack Down",
        "Silence The Sneak",
        "Arms Shortage",
        "Evidence Dash",
        "Gone Fishing",
        "Plaster Blaster",
        "Marked Man",
        "Liberator",
        "Waka-Gashira Wipeout!",
        "A Drop In The Ocean",
        "Bling-Bling Scramble",
        "Uzi Rider",
        "Gangcar Round-Up",
        "Kingdom Come",
        "Grand Theft Aero",
        "Escort Service",
        "Decoy",
        "Love's Disappearance",
        "Bait",
        "Espresso-2-Go!",
        "S.A.M.",
        "Uzi Money",
        "Toyminator",
        "Rigged To Blow",
        "Bullion Run",
        "Rumble",
        "The Exchange",
    };

    for (std::size_t i = 0; i < w.size(); ++i) {
        menu->lambda(w[i], [=] {
            ScriptMachine* vm = game->getScriptVM();

            if (vm) {
                std::list<SCMThread>& threads = vm->getThreads();
                const auto& offsets = vm->getFile()->getMissionOffsets();

                RW_ASSERT(!offsets.empty());

                for (auto& thread : threads) {
                    if (thread.baseAddress >= offsets[0]) {
                        thread.wakeCounter = -1;
                        thread.finished = true;
                    }
                }

                game->getState()->radarBlips.clear();

                RW_ASSERT(i < offsets.size());

                vm->startThread(offsets[i], true);
            }
        });
    }

    menu->offset = kDebugMenuOffset;
    return menu;
}

DebugState::DebugState(RWGame* game, const glm::vec3& vp, const glm::quat& vd)
    : State(game), _invertedY(game->getConfig().getInputInvertY()) {
    this->enterMenu(createDebugMenu());

    _debugCam.position = vp;
    _debugCam.rotation = vd;
}

void DebugState::enter() {
    getWindow().showCursor();
}

void DebugState::exit() {
}

void DebugState::tick(float dt) {
    if (_freeLook) {
        _debugCam.rotation =
            glm::angleAxis(_debugLook.x, glm::vec3(0.f, 0.f, 1.f)) *
            glm::angleAxis(_debugLook.y, glm::vec3(0.f, 1.f, 0.f));

        _debugCam.position +=
            _debugCam.rotation * _movement * dt * (_sonicMode ? 500.f : 50.f);
    }
}

void DebugState::draw(GameRenderer* r) {
    // Draw useful information like camera position.
    std::stringstream ss;
    ss << "Camera Position: " << glm::to_string(_debugCam.position) << "\n";
    auto zone = getWorld()->data->findZoneAt(_debugCam.position);
    ss << (zone ? zone->name : "No Zone") << "\n";

    TextRenderer::TextInfo ti;
    ti.font = FONT_ARIAL;
    ti.text = GameStringUtil::fromString(ss.str(), ti.font);
    ti.screenPosition = glm::vec2(10.f, 10.f);
    ti.size = 15.f;
    ti.baseColour = glm::u8vec3(255);
    r->text.renderText(ti);

    State::draw(r);
}

void DebugState::handleEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    done();
                    break;
                case SDLK_w:
                    _movement.x = 1.f;
                    break;
                case SDLK_s:
                    _movement.x = -1.f;
                    break;
                case SDLK_a:
                    _movement.y = 1.f;
                    break;
                case SDLK_d:
                    _movement.y = -1.f;
                    break;
                case SDLK_f:
                    _freeLook = !_freeLook;
                    if (_freeLook)
                        getWindow().hideCursor();
                    else
                        getWindow().showCursor();
                    break;
                case SDLK_p:
                    printCameraDetails();
                    break;
                case SDLK_LSHIFT:
                    _sonicMode = true;
                    break;
                default:
                    break;
            }

            break;

        case SDL_KEYUP:
            switch (event.key.keysym.sym) {
                case SDLK_w:
                case SDLK_s:
                    _movement.x = 0.f;
                    break;
                case SDLK_a:
                case SDLK_d:
                    _movement.y = 0.f;
                    break;
                case SDLK_LSHIFT:
                    _sonicMode = false;
                    break;
                default:
                    break;
            }

            break;

        case SDL_MOUSEMOTION:
            if (game->hasFocus()) {
                glm::ivec2 screenSize = getWindow().getSize();
                glm::vec2 mouseMove(
                    event.motion.xrel / static_cast<float>(screenSize.x),
                    event.motion.yrel / static_cast<float>(screenSize.y));

                if (!_invertedY) mouseMove.y = -mouseMove.y;

                _debugLook.x -= mouseMove.x;

                float qpi = glm::half_pi<float>();
                _debugLook.y -= glm::clamp(mouseMove.y, -qpi, qpi);
            }
            break;

        default:
            break;
    }
    State::handleEvent(event);
}

void DebugState::printCameraDetails() {
    std::cout << " " << _debugCam.position.x << " " << _debugCam.position.y
              << " " << _debugCam.position.z << " " << _debugCam.rotation.x
              << " " << _debugCam.rotation.y << " " << _debugCam.rotation.z
              << " " << _debugCam.rotation.w << std::endl;
}

void DebugState::spawnVehicle(unsigned int id) {
    auto ch = game->getWorld()->getPlayer()->getCharacter();
    if (!ch) return;

    auto playerRot = ch->getRotation();
    auto spawnPos = ch->getPosition();
    spawnPos += playerRot * glm::vec3(0.f, 3.f, 0.f);
    auto spawnRot = glm::quat(
        glm::vec3(0.f, 0.f, glm::roll(playerRot) + glm::half_pi<float>()));
    getWorld()->createVehicle(id, spawnPos, spawnRot);
}

void DebugState::spawnFollower(unsigned int id) {
    auto ch = game->getWorld()->getPlayer()->getCharacter();
    if (!ch) return;

    glm::vec3 fwd = ch->rotation * glm::vec3(0.f, 1.f, 0.f);

    glm::vec3 hit{}, normal{};
    if (game->hitWorldRay(ch->position + (fwd * 10.f), {0.f, 0.f, -2.f}, hit,
                          normal)) {
        auto spawnPos = hit + normal;
        auto follower = game->getWorld()->createPedestrian(id, spawnPos);
        jumpCharacter(game, follower, spawnPos);
        follower->controller->setGoal(CharacterController::FollowLeader);
        follower->controller->setTargetCharacter(ch);
    }
}

void DebugState::giveItem(int slot) {
    CharacterObject* player = nullptr;
    if (game->getWorld()->getPlayer()) {
        player = game->getWorld()->getPlayer()->getCharacter();
    }

    if (player) {
        player->addToInventory(slot, 100);
    }
}

const ViewCamera& DebugState::getCamera(float) {
    return _debugCam;
}
