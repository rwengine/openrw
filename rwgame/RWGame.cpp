#include "RWGame.hpp"

#include <glm/gtx/norm.hpp>

#include "RWImGui.hpp"
#include "GameInput.hpp"
#include "State.hpp"
#include "StateManager.hpp"
#include "states/BenchmarkState.hpp"
#include "states/IngameState.hpp"
#include "states/LoadingState.hpp"
#include "states/MenuState.hpp"

#include <core/Profiler.hpp>

#include <engine/Payphone.hpp>
#include <engine/SaveGame.hpp>
#include <objects/GameObject.hpp>

#include <script/SCMFile.hpp>

#include <ai/AIGraphNode.hpp>
#include <ai/PlayerController.hpp>
#include <core/Logger.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/VehicleObject.hpp>

#include <boost/algorithm/string/predicate.hpp>
#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>

#ifdef _MSC_VER
#pragma warning(disable : 4305 5033)
#endif
// FIXME: should be in rwengine, deeply hidden
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#ifdef _MSC_VER
#pragma warning(default : 4305 5033)
#endif

namespace {
static constexpr std::array<
    std::tuple<GameRenderer::SpecialModel, char const*, char const*>, 3>
    kSpecialModels{{{GameRenderer::ZoneCylinderA, "zonecyla.dff", "particle"},
                    {GameRenderer::ZoneCylinderB, "zonecylb.dff", "particle"},
                    {GameRenderer::Arrow, "arrow.dff", ""}}};

constexpr float kMaxPhysicsSubSteps = 2;
}  // namespace

#define MOUSE_SENSITIVITY_SCALE 2.5f

RWGame::RWGame(Logger& log, const std::optional<RWArgConfigLayer> &args)
    : GameBase(log, args)
    , data(&log, config.gamedataPath())
    , renderer(&log, &data)
    , imgui(*this) {
    RW_PROFILE_THREAD("Main");
    RW_TIMELINE_ENTER("Startup", MP_YELLOW);

    auto loadTimeStart = std::chrono::steady_clock::now();
    bool newgame = false;
    bool test = false;
    std::optional<std::string> startSave;
    std::optional<std::string> benchFile;
    if (args.has_value()) {
        newgame = args->newGame;
        test = args->test;
        startSave = args->loadGamePath;
        benchFile = args->benchmarkPath;
    }

    imgui.init();

    log.info("Game", "Game directory: " + config.gamedataPath());
    if (!data.load()) {
        throw std::runtime_error("Invalid game directory path: " +
                                 config.gamedataPath());
    }

    for (const auto& [specialModel, fileName, name] : kSpecialModels) {
        auto model = data.loadClump(fileName, name);
        renderer.setSpecialModel(specialModel, model);
    }

    // Set up text renderer
    renderer.text.setFontTexture(FONT_PAGER, "pager");
    renderer.text.setFontTexture(FONT_PRICEDOWN, "font1");
    renderer.text.setFontTexture(FONT_ARIAL, "font2");

    hudDrawer.applyHUDScale(config.hudScale());
    renderer.map.scaleHUD(config.hudScale());

    debug.setDebugMode(btIDebugDraw::DBG_DrawWireframe |
                       btIDebugDraw::DBG_DrawConstraints |
                       btIDebugDraw::DBG_DrawConstraintLimits);
    debug.setShaderProgram(renderer.worldProg.get());

    data.loadDynamicObjects((std::filesystem::path{config.gamedataPath()} / "data/object.dat")
                                .string());  // FIXME: use path

    data.loadGXT("text/" + config.gameLanguage() + ".gxt");

    getRenderer().water.setWaterTable(data.waterHeights, 48, data.realWater,
                                      128 * 128);

    for (int m = 0; m < MAP_BLOCK_SIZE; ++m) {
        std::ostringstream oss;
        oss << "radar" << std::setw(2) << std::setfill('0') << m << ".txd";
        data.loadTXD(oss.str());
    }

    stateManager.enter<LoadingState>(this, [=]() {
        if (benchFile.has_value()) {
            stateManager.enter<BenchmarkState>(this, *benchFile);
        } else if (test) {
            stateManager.enter<IngameState>(this, true, "test");
        } else if (newgame) {
            stateManager.enter<IngameState>(this, true);
        } else if (startSave.has_value()) {
            stateManager.enter<IngameState>(this, true, *startSave);
        } else {
            stateManager.enter<MenuState>(this);
        }
    });

    auto loadTimeEnd = std::chrono::steady_clock::now();
    auto loadTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(loadTimeEnd - loadTimeStart);
    log.info("Game", "Loading took " + std::to_string(loadTime.count()) + " ms");

    log.info("Game", "Started");
    RW_TIMELINE_LEAVE("Startup");
}

RWGame::~RWGame() {
    log.info("Game", "Beginning cleanup");
}

void RWGame::newGame() {
    // Get a fresh state
    state = GameState();

    // Destroy the current world and start over
    world = std::make_unique<GameWorld>(&log, &data);
    world->dynamicsWorld->setDebugDrawer(&debug);

    // Associate the new world with the new state and vice versa
    state.world = world.get();
    world->state = &state;

    for (auto ipl : world->data->iplLocations) {
        world->data->loadZone(ipl.second);
        world->placeItems(ipl.second);
    }
}

bool RWGame::hitWorldRay(glm::vec3 &hit, glm::vec3 &normal, GameObject **object) {
    auto vc = currentCam;
    glm::vec3 from(vc.position.x, vc.position.y, vc.position.z);
    glm::vec3 tmp = vc.rotation * glm::vec3(1000.f, 0.f, 0.f);

    return hitWorldRay(from, tmp, hit, normal, object);
}

bool RWGame::hitWorldRay(const glm::vec3 &start, const glm::vec3 &direction, glm::vec3 &hit, glm::vec3 &normal, GameObject **object) {
    auto from = btVector3(start.x, start.y, start.z);
    auto to = btVector3(start.x + direction.x, start.y + direction.y,
                        start.z + direction.z);
    btCollisionWorld::ClosestRayResultCallback ray(from, to);

    world->dynamicsWorld->rayTest(from, to, ray);
    if (ray.hasHit()) {
        hit = glm::vec3(ray.m_hitPointWorld.x(), ray.m_hitPointWorld.y(),
                        ray.m_hitPointWorld.z());
        normal =
                glm::vec3(ray.m_hitNormalWorld.x(), ray.m_hitNormalWorld.y(),
                          ray.m_hitNormalWorld.z());
        if (object) {
            *object = static_cast<GameObject*>(
                        ray.m_collisionObject->getUserPointer());
        }
        return true;
    }
    return false;
}

void RWGame::saveGame(const std::string& savename) {
    RW_UNUSED(savename);
}

void RWGame::loadGame(const std::string& savename) {
    delete state.script;

    log.info("Game", "Loading game " + savename);

    newGame();

    startScript("data/main.scm");

    if (!SaveGame::loadGame(state, savename)) {
        log.error("Game", "Failed to load game");
    }

    // Set fade splash
    state.world->data->loadSplash("SPLASH1");
}

void RWGame::startScript(const std::string& name) {
    script = data.loadSCM(name);
    if (script) {
        vm = std::make_unique<ScriptMachine>(&state, script, &opcodes);
        state.script = vm.get();
    } else {
        log.error("Game", "Failed to load SCM: " + name);
    }
}

// Modifiers for GTA3 we try to recreate
#define RW_GAME_VERSION 1100
#define RW_GAME_GTA3_GERMAN 0
#define RW_GAME_GTA3_ANNIVERSARY 0

void RWGame::handleCheatInput(char symbol) {
    cheatInputWindow = cheatInputWindow.substr(1) + symbol;

    // Helper to check for cheats
    auto checkForCheat = [this](const std::string& cheat,
                                std::function<void()> action) {
        RW_CHECK(cheatInputWindow.length() >= cheat.length(), "Cheat too long");
        size_t offset = cheatInputWindow.length() - cheat.length();
        if (cheat == cheatInputWindow.substr(offset)) {
            log.info("Game", "Cheat triggered: '" + cheat + "'");
            if (action) {
                action();
            }
        }
    };

    // Player related cheats
    {
        auto player = getWorld()->getPlayer()->getCharacter();

#ifdef RW_GAME_GTA3_GERMAN  // Germans got their own cheat
        std::string health_cheat = "GESUNDHEIT";
#else
        std::string health_cheat = "HEALTH";
#endif
        checkForCheat(health_cheat, [&] {
            player->getCurrentState().health = 100.f;
            state.showHelpMessage("CHEAT3"); // III / VC: Inputting health cheat.
        });

#if RW_GAME_VERSION >= 1100  // Changed cheat name in version 1.1
        std::string armor_cheat = "TORTOISE";
#else
        std::string armor_cheat = "TURTOISE";
#endif
        checkForCheat(armor_cheat, [&] {
            player->getCurrentState().armour = 100.f;
            state.showHelpMessage("CHEAT4"); // III / VC: Inputting armor cheat.
        });

        checkForCheat("GUNSGUNSGUNS", [&] {
            static constexpr std::array<int, 11> ammo = {{
                1,  //baseball bat
                100,//pistol
                100,//uzi
                20, //shotgun
                5,  //grenade
                5,  //molotov
                5,  //rocket launcher
                20, //flamethrower
                200,//ak47
                200,//m16
                5   //sniper rifle
            }};
            for (auto i = 0u; i < ammo.size(); i++)
                player->addToInventory(static_cast<int>(i+1),ammo[i]);
            state.showHelpMessage("CHEAT2"); // III / VC: Inputting weapon cheats.
        });

        checkForCheat("IFIWEREARICHMAN", [&] {
            state.playerInfo.money += 250000;
            state.showHelpMessage("CHEAT6"); // III: Inputting money cheat.
        });

        checkForCheat("MOREPOLICEPLEASE", [&] {
            // @todo raise to next wanted level
            state.showHelpMessage("CHEAT5"); // III / VC: Inputting wanted level cheats.
        });

        checkForCheat("NOPOLICEPLEASE", [&] {
            // @todo lower to next lower wanted level
            state.showHelpMessage("CHEAT5"); // III / VC: Inputting wanted level cheats.
        });
    }

    // Misc cheats.
    {
        checkForCheat("BANGBANGBANG", [&] {
            // @todo Explode nearby vehicles
            // @todo What radius?
            state.showHelpMessage("CHEAT1"); // III / VC: Inputting most cheats.
        });

        checkForCheat("GIVEUSATANK", [&] {
// The iPod / Android version of the game (10th year anniversary) spawns random
// (?) vehicles instead of always rhino
#if RW_GAME_GTA3_ANNIVERSARY != 0
         uint16_t vehicleModel = 110;  // @todo Which cars are spawned?!
#else
         uint16_t vehicleModel = 122;
#endif
            const auto ch = getWorld()->getPlayer()->getCharacter();// @todo Change spawn place to be more like in original game
            const auto playerRot = ch->getRotation();
            const auto spawnPos = ch->getPosition() + playerRot * glm::vec3(0.f, 3.f, 0.f);
            const auto spawnRot = glm::quat(
                    glm::vec3(0.f, 0.f, glm::roll(playerRot) + glm::half_pi<float>()));
            getWorld()->createVehicle(vehicleModel, spawnPos, spawnRot);
            state.showHelpMessage("CHEAT1"); // III / VC: Inputting most cheats.
        });

        checkForCheat("CORNERSLIKEMAD", [&] {
            // @todo Weird car handling
            state.showHelpMessage("CHEAT1"); // III / VC: Inputting most cheats.
        });

        checkForCheat("ANICESETOFWHEELS", [&] {
            // @todo Hide car bodies
            state.showHelpMessage("CHEAT1"); // III / VC: Inputting most cheats.
        });

        checkForCheat("CHITTYCHITTYBB", [&] {
            // @todo Cars can fly
            state.showHelpMessage("CHEAT1"); // III / VC: Inputting most cheats.
        });

        checkForCheat("NASTYLIMBCHEAT", [&] {
            // @todo Makes it possible to shoot limbs off, iirc?
            state.showHelpMessage("CHEAT1"); // III / VC: Inputting most cheats.
        });

        checkForCheat("ILIKEDRESSINGUP", [&] {
            // @todo Which skins will be chosen?
            state.showHelpMessage("CHEAT1"); // III / VC: Inputting most cheats.
        });
    }

    // Pedestrian cheats
    {
        checkForCheat("WEAPONSFORALL", [&] {
            // @todo Give all pedestrians weapons.. this is also saved in the
            // savegame?!
            // @todo Which weapons do they get?
            state.showHelpMessage("CHEAT1"); // III / VC: Inputting most cheats.
        });

        checkForCheat("NOBODYLIKESME", [&] {
            // @todo Set all pedestrians hostile towards player.. this is also
            // saved in the savegame?!
            state.showHelpMessage("CHEAT1"); // III / VC: Inputting most cheats.
        });

        checkForCheat("ITSALLGOINGMAAAD", [&] {
            // @todo Set all pedestrians to fighting each other.. this is also
            // saved in the savegame?!
            state.showHelpMessage("CHEAT1"); // III / VC: Inputting most cheats.
        });

        // Game speed cheats

        checkForCheat("TIMEFLIESWHENYOU", [&] {
            // @todo Set fast gamespeed
            state.showHelpMessage("CHEAT1"); // III / VC: Inputting most cheats.
        });

        checkForCheat("BOOOOORING", [&] {
            // @todo Set slow gamespeed
            state.showHelpMessage("CHEAT1"); // III / VC: Inputting most cheats.
        });
    }

    // Weather cheats
    {
        checkForCheat("ILIKESCOTLAND", [&] {
            // @todo Set weather to cloudy
            state.showHelpMessage("CHEAT7"); // III / VC: Inputting weather cheats.
        });

        checkForCheat("SKINCANCERFORME", [&] {
            // @todo Set sunny / normal weather
            state.showHelpMessage("CHEAT7"); // III / VC: Inputting weather cheats.
        });

        checkForCheat("MADWEATHER", [&] {
            // @todo Set bad weather
            state.showHelpMessage("CHEAT7"); // III / VC: Inputting weather cheats.
        });

        checkForCheat("ILOVESCOTLAND", [&] {
            // @todo Set weather to rainy
            state.showHelpMessage("CHEAT7"); // III / VC: Inputting weather cheats.
        });

        checkForCheat("PEASOUP", [&] {
            // @todo Set weather to foggy
            state.showHelpMessage("CHEAT7"); // III / VC: Inputting weather cheats.
        });
    }
}

int RWGame::run() {
    namespace chrono = std::chrono;

    auto lastFrame = chrono::steady_clock::now();
    const float deltaTime = GAME_TIMESTEP;
    float accumulatedTime = 0.0f;

    // Loop until we run out of states.
    bool running = true;
    while (stateManager.currentState() && running) {
        RW_PROFILE_FRAME_BOUNDARY();
        RW_PROFILE_SCOPE("Main Loop");

        running = updateInput();

        auto currentFrame = chrono::steady_clock::now();
        auto frameTime =
            chrono::duration<float>(currentFrame - lastFrame).count();
        lastFrame = currentFrame;

        if (!world->isPaused()) {
            accumulatedTime += frameTime;

            // Clamp frameTime, so we won't freeze completely
            if (frameTime > 0.1f) {
                frameTime = 0.1f;
            }

            accumulatedTime = tickWorld(deltaTime, accumulatedTime);
        }

        render(1, frameTime);

        getWindow().swap();

        // Make sure the topmost state is the correct state
        stateManager.updateStack();
    }

    stateManager.clear();

    return 0;
}

float RWGame::tickWorld(const float deltaTime, float accumulatedTime) {
    RW_PROFILE_SCOPEC(__func__, MP_GREEN);
    auto deltaTimeWithTimeScale =
            deltaTime * world->state->basic.timeScale;

    while (accumulatedTime >= deltaTime) {
        if (!stateManager.currentState()) {
            break;
        }

        {
            RW_PROFILE_SCOPEC("stepSimulation", MP_DARKORANGE1);
            world->dynamicsWorld->stepSimulation(
                    deltaTimeWithTimeScale, kMaxPhysicsSubSteps, deltaTime);
        }

        stateManager.tick(deltaTimeWithTimeScale);

        tick(deltaTimeWithTimeScale);

        getState()->swapInputState();

        accumulatedTime -= deltaTime;
    }
    return accumulatedTime;
}

bool RWGame::updateInput() {
    RW_PROFILE_SCOPE(__func__);
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (imgui.processEvent(event)) {
            continue;
        }
        switch (event.type) {
            case SDL_QUIT:
                return false;

            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                        inFocus = true;
                        break;

                    case SDL_WINDOWEVENT_FOCUS_LOST:
                        inFocus = false;
                        break;
                }
                break;

            case SDL_KEYDOWN:
                globalKeyEvent(event);
                break;

            case SDL_MOUSEMOTION:
                event.motion.xrel *= MOUSE_SENSITIVITY_SCALE;
                event.motion.yrel *= MOUSE_SENSITIVITY_SCALE;
                break;
        }

        GameInput::updateGameInputState(&getState()->input[0], event);

        if (stateManager.currentState()) {
            RW_PROFILE_SCOPE("State");
            stateManager.currentState()->handleEvent(event);
        }
    }
    return true;
}

void RWGame::tick(float dt) {
    RW_PROFILE_SCOPE(__func__);
    State* currState = stateManager.states.back().get();

    static float clockAccumulator = 0.f;
    static float scriptTimerAccumulator = 0.f;
    static ScriptInt beepTime = std::numeric_limits<ScriptInt>::max();
    if (currState->shouldWorldUpdate()) {
        world->chase.update(dt);

        // Clear out any per-tick state.
        world->clearTickData();

        state.gameTime += dt;

        clockAccumulator += dt;
        while (clockAccumulator >= 1.f) {
            state.basic.gameMinute++;
            while (state.basic.gameMinute >= 60) {
                state.basic.gameMinute = 0;
                state.basic.gameHour++;
                while (state.basic.gameHour >= 24) {
                    state.basic.gameHour = 0;
                }
            }
            clockAccumulator -= 1.f;
        }

        constexpr float timerClockRate = 1.f / 30.f;

        if (state.scriptTimerVariable && !state.scriptTimerPaused) {
            scriptTimerAccumulator += dt;
            while (scriptTimerAccumulator >= timerClockRate) {
                // Original game uses milliseconds
                (*state.scriptTimerVariable) -= timerClockRate * 1000;

                //                                11 seconds
                if (*state.scriptTimerVariable <= 11000 &&
                    beepTime - *state.scriptTimerVariable >= 1000) {
                    beepTime = *state.scriptTimerVariable;

                    // @todo beep
                }

                if (*state.scriptTimerVariable <= 0) {
                    (*state.scriptTimerVariable) = 0;
                    state.scriptTimerVariable = nullptr;
                }

                scriptTimerAccumulator -= timerClockRate;
            }
        }

        tickObjects(dt);

        state.text.tick(dt);

        if (vm) {
            try {
                vm->execute(dt);
            } catch (SCMException& ex) {
                std::cerr << ex.what() << '\n';
                log.error("Script", ex.what());
                throw;
            }
        }

        /// @todo this doesn't make sense as the condition
        if (state.playerObject) {
            currentCam.frustum.update(currentCam.frustum.projection() *
                                      currentCam.getView());
            // Use the current camera position to spawn pedestrians.
            world->cleanupTraffic(currentCam);
            // Only create new traffic outside cutscenes
            if (!state.currentCutscene) {
                world->createTraffic(currentCam);
            }
        }
    }
}

void RWGame::tickObjects(float dt) const {
    RW_PROFILE_SCOPEC(__func__, MP_MAGENTA1);
    world->updateEffects();

    {
        RW_PROFILE_SCOPEC("allObjects", MP_HOTPINK1);
        RW_PROFILE_COUNTER_SET("tickObjects/allObjects", world->allObjects.size());
        for (auto &object : world->allObjects) {
            object->tick(dt);
        }
    }

    {
        RW_PROFILE_SCOPEC("garages", MP_HOTPINK2);
        for (auto &g : world->garages) {
            g->tick(dt);
        }
    }

    {
        RW_PROFILE_SCOPEC("payphones", MP_HOTPINK3);
        for (auto &p : world->payphones) {
            p->tick(dt);
        }
    }

    world->destroyQueuedObjects();
}

void RWGame::render(float alpha, float time) {
    RW_PROFILE_SCOPEC(__func__, MP_CORNFLOWERBLUE);
    RW_UNUSED(time);

    lastDraws = getRenderer().getRenderer().getDrawCount();

    getRenderer().getRenderer().swap();
    imgui.startFrame();

    // Update the camera
    if (!stateManager.states.empty()) {
        currentCam = stateManager.states.back()->getCamera(alpha);
    }

    glm::ivec2 windowSize = getWindow().getSize();
    renderer.setViewport(windowSize.x, windowSize.y);

    ViewCamera viewCam = currentCam;

    viewCam.frustum.aspectRatio =
        windowSize.x / static_cast<float>(windowSize.y);

    if (state.isCinematic) {
        viewCam.frustum.fov *= viewCam.frustum.aspectRatio;
    }

    world->sound.updateListenerTransform(viewCam);

    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    renderer.getRenderer().pushDebugGroup("World");

    renderer.renderWorld(world.get(), viewCam, alpha);

    renderer.getRenderer().popDebugGroup();

    renderDebugView();

    if (!world->isPaused()) hudDrawer.drawOnScreenText(world.get(), renderer);

    if (stateManager.currentState()) {
        RW_PROFILE_SCOPE("state");
        stateManager.draw(renderer);
    }

    imgui.endFrame(viewCam);
}

void RWGame::renderDebugView() {
    RW_PROFILE_SCOPE(__func__);
    switch (debugview_) {
        case DebugViewMode::Physics:
            world->dynamicsWorld->debugDrawWorld();
            debug.flush(renderer);
            break;
        case DebugViewMode::Navigation:
            renderDebugPaths();
            break;
        default:
            break;
    }
}

void RWGame::renderDebugPaths() {
    btVector3 roadColour(1.f, 0.f, 0.f);
    btVector3 pedColour(0.f, 0.f, 1.f);

    for (const auto& n : world->aigraph.nodes) {
        btVector3 p(n->position.x, n->position.y, n->position.z);
        auto& col = n->type == ai::NodeType::Pedestrian ? pedColour : roadColour;
        debug.drawLine(p - btVector3(0.f, 0.f, 1.f),
                       p + btVector3(0.f, 0.f, 1.f), col);
        debug.drawLine(p - btVector3(1.f, 0.f, 0.f),
                       p + btVector3(1.f, 0.f, 0.f), col);
        debug.drawLine(p - btVector3(0.f, 1.f, 0.f),
                       p + btVector3(0.f, 1.f, 0.f), col);

        for (const auto& c : n->connections) {
            btVector3 f(c->position.x, c->position.y, c->position.z);
            debug.drawLine(p, f, col);
        }
    }

    // Draw Garage bounds
    for (const auto& garage : world->garages) {
        btVector3 minColor(1.f, 0.f, 0.f);
        btVector3 maxColor(0.f, 1.f, 0.f);
        btVector3 min(garage->min.x, garage->min.y, garage->min.z);
        btVector3 max(garage->max.x, garage->max.y, garage->max.z);
        debug.drawLine(min, min + btVector3(0.5f, 0.f, 0.f), minColor);
        debug.drawLine(min, min + btVector3(0.f, 0.5f, 0.f), minColor);
        debug.drawLine(min, min + btVector3(0.f, 0.f, 0.5f), minColor);

        debug.drawLine(max, max - btVector3(0.5f, 0.f, 0.f), maxColor);
        debug.drawLine(max, max - btVector3(0.f, 0.5f, 0.f), maxColor);
        debug.drawLine(max, max - btVector3(0.f, 0.f, 0.5f), maxColor);
    }

    // Draw vehicle generators
    for (const auto& generator : state.vehicleGenerators) {
        btVector3 color(1.f, 0.f, 0.f);
        btVector3 position(generator.position.x, generator.position.y,
                           generator.position.z);
        float heading = glm::radians(generator.heading);
        auto back =
            btVector3(0.f, -1.f, 0.f).rotate(btVector3(0.f, 0.f, 1.f), heading);
        auto right = btVector3(0.15f, -0.15f, 0.f)
                         .rotate(btVector3(0.f, 0.f, 1.f), heading);
        auto left = btVector3(-0.15f, -0.15f, 0.f)
                        .rotate(btVector3(0.f, 0.f, 1.f), heading);
        debug.drawLine(position, position + back, color);
        debug.drawLine(position, position + right, color);
        debug.drawLine(position, position + left, color);
    }

    // Draw the targetNode if a character is driving a vehicle
    for (auto& p : world->pedestrianPool.objects) {
        auto v = static_cast<CharacterObject*>(p.second.get());

        static const glm::vec3 color(1.f, 1.f, 0.f);

        if (auto vehicle = v->getCurrentVehicle(); vehicle)
        {
            if (v->controller->targetNode) {
                debug.drawLine(v->getPosition(), v->controller->targetNode->position, color);
            }

            auto [center, halfSize] = vehicle->obstacleCheckVolume();
            std::array<glm::vec3, 8> corners { {
                    glm::vec3{- halfSize.x, - halfSize.y, - halfSize.z},
                    glm::vec3{+ halfSize.x, - halfSize.y, - halfSize.z},
                    glm::vec3{- halfSize.x, - halfSize.y, + halfSize.z},
                    glm::vec3{+ halfSize.x, - halfSize.y, + halfSize.z},
                    glm::vec3{- halfSize.x, + halfSize.y, - halfSize.z},
                    glm::vec3{+ halfSize.x, + halfSize.y, - halfSize.z},
                    glm::vec3{- halfSize.x, + halfSize.y, + halfSize.z},
                    glm::vec3{+ halfSize.x, + halfSize.y, + halfSize.z},
                }
            };
            const auto iRotation = (vehicle->getRotation());
            const auto rCenter = iRotation * center;
            std::transform(corners.begin(), corners.end(), corners.begin(),
                           [&](const auto& p) -> glm::vec3 {
                               return vehicle->getPosition() + rCenter + iRotation * p;
                           });

            static const glm::vec3 color2(1.f, 0.f, 0.f);
            debug.drawLine(corners[0], corners[1], color2);
            debug.drawLine(corners[0], corners[2], color2);
            debug.drawLine(corners[3], corners[1], color2);
            debug.drawLine(corners[3], corners[2], color2);

            debug.drawLine(corners[0], corners[4], color2);
            debug.drawLine(corners[1], corners[5], color2);
            debug.drawLine(corners[2], corners[6], color2);
            debug.drawLine(corners[3], corners[7], color2);

            debug.drawLine(corners[4], corners[5], color2);
            debug.drawLine(corners[4], corners[6], color2);
            debug.drawLine(corners[7], corners[5], color2);
            debug.drawLine(corners[7], corners[6], color2);
        }
    }

    debug.flush(renderer);
}

void RWGame::globalKeyEvent(const SDL_Event& event) {
    const auto toggle_debug = [&](DebugViewMode m) {
        debugview_ = debugview_ == m ? DebugViewMode::Disabled : m;
    };

    switch (event.key.keysym.sym) {
        case SDLK_LEFTBRACKET:
            world->offsetGameTime(-30);
            break;
        case SDLK_RIGHTBRACKET:
            world->offsetGameTime(30);
            break;
        case SDLK_9:
            world->state->basic.timeScale *= 0.5f;
            break;
        case SDLK_0:
            world->state->basic.timeScale *= 2.0f;
            break;
        case SDLK_F1:
            toggle_debug(DebugViewMode::General);
            break;
        case SDLK_F2:
            toggle_debug(DebugViewMode::Navigation);
            break;
        case SDLK_F3:
            toggle_debug(DebugViewMode::Physics);
            break;
        case SDLK_F4:
            toggle_debug(DebugViewMode::Objects);
            break;
        default:
            break;
    }

    std::string keyName = SDL_GetKeyName(event.key.keysym.sym);
    if (getWorld()->getPlayer() && keyName.length() == 1) {
        char symbol = keyName[0];
        handleCheatInput(symbol);
    }
}
