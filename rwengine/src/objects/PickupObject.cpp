#include "objects/PickupObject.hpp"

#include <cmath>

#ifdef _MSC_VER
#pragma warning(disable : 4305)
#endif
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <btBulletDynamicsCommon.h>
#ifdef _MSC_VER
#pragma warning(default : 4305)
#endif

#include "ai/PlayerController.hpp"
#include "data/WeaponData.hpp"
#include "engine/GameData.hpp"
#include "engine/GameState.hpp"
#include "engine/GameWorld.hpp"
#include "engine/ScreenText.hpp"
#include "objects/CharacterObject.hpp"
#include "objects/PickupObject.hpp"
#include "objects/VehicleObject.hpp"

namespace {
const std::array<glm::vec3, 106> kBigNVeinyPickupsLocations = {{
    glm::vec3(913.62219f, -155.13692f, 4.9699469f),
    glm::vec3(913.92401f, -124.12943f, 4.9692569f),
    glm::vec3(913.27899f, -93.524231f, 7.4325991f),
    glm::vec3(912.60852f, -63.15905f, 7.4533591f),
    glm::vec3(934.22144f, -42.049122f, 7.4511471f),
    glm::vec3(958.88092f, -23.863735f, 7.4652338f),
    glm::vec3(978.50812f, -0.78458798f, 5.13515f),
    glm::vec3(1009.4175f, -2.1041219f, 2.4461579f),
    glm::vec3(1040.6313f, -2.0793829f, 2.293175f),
    glm::vec3(1070.7863f, -2.084095f, 2.2789791f),
    glm::vec3(1100.5773f, -8.468729f, 5.3248072f),
    glm::vec3(1119.9341f, -31.738031f, 7.1913071f),
    glm::vec3(1122.1664f, -62.762737f, 7.4703908f),
    glm::vec3(1122.814f, -93.650566f, 8.5577497f),
    glm::vec3(1125.8253f, -124.26616f, 9.9803305f),
    glm::vec3(1153.8727f, -135.47169f, 14.150617f),
    glm::vec3(1184.0831f, -135.82845f, 14.973998f),
    glm::vec3(1192.0432f, -164.57816f, 19.18627f),
    glm::vec3(1192.7761f, -194.28871f, 24.799675f),
    glm::vec3(1215.1527f, -215.0714f, 25.74975f),
    glm::vec3(1245.79f, -215.39304f, 28.70726f),
    glm::vec3(1276.2477f, -216.39485f, 33.71236f),
    glm::vec3(1306.5535f, -216.71007f, 39.711472f),
    glm::vec3(1335.0244f, -224.59329f, 46.474979f),
    glm::vec3(1355.4879f, -246.27664f, 49.934841f),
    glm::vec3(1362.6003f, -276.47064f, 49.96265f),
    glm::vec3(1363.027f, -307.30847f, 49.969173f),
    glm::vec3(1365.343f, -338.08609f, 49.967789f),
    glm::vec3(1367.5957f, -368.01105f, 50.092304f),
    glm::vec3(1368.2749f, -398.38049f, 50.061268f),
    glm::vec3(1366.9034f, -429.98483f, 50.057545f),
    glm::vec3(1356.8534f, -459.09259f, 50.035545f),
    glm::vec3(1335.5819f, -481.13544f, 47.217903f),
    glm::vec3(1306.7552f, -491.07443f, 40.202629f),
    glm::vec3(1275.5978f, -491.33194f, 33.969223f),
    glm::vec3(1244.702f, -491.46451f, 29.111021f),
    glm::vec3(1213.2222f, -491.8754f, 25.771168f),
    glm::vec3(1182.7729f, -492.19995f, 24.749964f),
    glm::vec3(1152.6874f, -491.42221f, 21.70038f),
    glm::vec3(1121.5352f, -491.94604f, 20.075182f),
    glm::vec3(1090.7056f, -492.63751f, 17.585758f),
    glm::vec3(1059.6008f, -491.65762f, 14.848632f),
    glm::vec3(1029.113f, -489.66031f, 14.918498f),
    glm::vec3(998.20679f, -486.78107f, 14.945688f),
    glm::vec3(968.00555f, -484.91266f, 15.001229f),
    glm::vec3(937.74939f, -492.09015f, 14.958629f),
    glm::vec3(927.17352f, -520.97736f, 14.972308f),
    glm::vec3(929.29749f, -552.08643f, 14.978855f),
    glm::vec3(950.69525f, -574.47778f, 14.972788f),
    glm::vec3(974.02826f, -593.56024f, 14.966445f),
    glm::vec3(989.04779f, -620.12854f, 14.951016f),
    glm::vec3(1014.1639f, -637.3905f, 14.966736f),
    glm::vec3(1017.5961f, -667.3736f, 14.956415f),
    glm::vec3(1041.9735f, -685.94391f, 15.003841f),
    glm::vec3(1043.3064f, -716.11298f, 14.974236f),
    glm::vec3(1043.5337f, -746.63855f, 14.96919f),
    glm::vec3(1044.142f, -776.93823f, 14.965424f),
    glm::vec3(1044.2657f, -807.29395f, 14.97171f),
    glm::vec3(1017.0797f, -820.1076f, 14.975431f),
    glm::vec3(986.23865f, -820.37103f, 14.972883f),
    glm::vec3(956.10065f, -820.23291f, 14.981133f),
    glm::vec3(925.86914f, -820.19049f, 14.976553f),
    glm::vec3(897.69702f, -831.08734f, 14.962709f),
    glm::vec3(868.06586f, -835.99237f, 14.970685f),
    glm::vec3(836.93054f, -836.84387f, 14.965049f),
    glm::vec3(811.63586f, -853.7915f, 15.067576f),
    glm::vec3(811.46344f, -884.27368f, 12.247812f),
    glm::vec3(811.60651f, -914.70959f, 9.2393751f),
    glm::vec3(811.10425f, -945.16272f, 5.817255f),
    glm::vec3(816.54584f, -975.64587f, 4.998558f),
    glm::vec3(828.2951f, -1003.3685f, 5.0471172f),
    glm::vec3(852.28839f, -1021.5963f, 4.9371028f),
    glm::vec3(882.50067f, -1025.4459f, 5.14077f),
    glm::vec3(912.84821f, -1026.7874f, 8.3415451f),
    glm::vec3(943.68274f, -1026.6914f, 11.341879f),
    glm::vec3(974.4129f, -1027.3682f, 14.410345f),
    glm::vec3(1004.1079f, -1036.0778f, 14.92961f),
    glm::vec3(1030.1144f, -1051.1224f, 14.850387f),
    glm::vec3(1058.7585f, -1060.342f, 14.821624f),
    glm::vec3(1087.7797f, -1068.3263f, 14.800561f),
    glm::vec3(1099.8807f, -1095.656f, 11.877907f),
    glm::vec3(1130.0005f, -1101.994f, 11.853914f),
    glm::vec3(1160.3809f, -1101.6355f, 11.854824f),
    glm::vec3(1191.8524f, -1102.1577f, 11.853843f),
    glm::vec3(1223.3307f, -1102.7448f, 11.852233f),
    glm::vec3(1253.564f, -1098.1045f, 11.853944f),
    glm::vec3(1262.0203f, -1069.1785f, 14.8147f),
    glm::vec3(1290.9998f, -1059.1882f, 14.816016f),
    glm::vec3(1316.246f, -1041.0635f, 14.81109f),
    glm::vec3(1331.7539f, -1013.835f, 14.81207f),
    glm::vec3(1334.0579f, -983.55402f, 14.827253f),
    glm::vec3(1323.2429f, -954.23083f, 14.954678f),
    glm::vec3(1302.7495f, -932.21216f, 14.962917f),
    glm::vec3(1317.418f, -905.89325f, 14.967506f),
    glm::vec3(1337.9503f, -883.5025f, 14.969675f),
    glm::vec3(1352.6929f, -855.96954f, 14.967854f),
    glm::vec3(1357.2388f, -826.26971f, 14.97295f),
    glm::vec3(1384.8668f, -812.47693f, 12.907736f),
    glm::vec3(1410.8983f, -795.39056f, 12.052228f),
    glm::vec3(1433.901f, -775.55811f, 11.96265f),
    glm::vec3(1443.8615f, -746.92511f, 11.976114f),
    glm::vec3(1457.7015f, -720.00903f, 11.971177f),
    glm::vec3(1481.5685f, -701.30237f, 11.977908f),
    glm::vec3(1511.4004f, -696.83295f, 11.972709f),
    glm::vec3(1542.1796f, -695.61676f, 11.970441f),
    glm::vec3(1570.3301f, -684.6239f, 11.969202f),
}};
uint32_t colours[14] = {
    0xff0000,  // bat, detonator, adrenaline
    0x00ff00,  // pistol
    0x8080ff,  // uzi
    0xffff00,  // shotgun
    0xff00ff,  // ak47
    0x00ffff,  // m16
    0xff8000,  // sniper, donkeymag
    0x00ff80,  // rocket
    0x8000ff,  // flame
    0x80ff00,  // molotov
    0xffffff,  // grenade. package1, floatpackge1
    0x80ff80,  // bodyarmour, bribe
    0x0000ff,  // info, killfrenzy
    0xffff00,  // health, bonus
};
}  // namespace

bool PickupObject::defaultDoesRespawn(PickupType type) {
    switch (type) {
        case Once:
        case OnceTimeout:
        case Collectable:
        case OutOfStock:
        case Money:
        case MineInactive:
        case MineArmed:
        case NauticalMineInactive:
        case NauticalMineArmed:
        case FloatingPackage:
        case FloatingPackageFloating:
            return false;
        default:
            return true;
    }
}

float PickupObject::defaultRespawnTime(PickupType type) {
    switch (type) {
        case InShop:
            return 5.f;
        case OnStreet:
        case OnStreetSlow:
            return 30.f;
        default:
            return 30.f;
    }
}

PickupObject::BehaviourFlags PickupObject::defaultBehaviourFlags(
    PickupType type) {
    switch (type) {
        case InShop:
        case OnStreet:
        case Once:
        case OnceTimeout:
        case Collectable:
        case Money:
        case OnStreetSlow:
            return PickupOnFoot;
        case OutOfStock:
        case MineInactive:
        case MineArmed:
        case NauticalMineInactive:
        case NauticalMineArmed:
            return None;
        case FloatingPackage:
        case FloatingPackageFloating:
            return PickupInVehicle;
        default:
            return None;
    }
}

PickupObject::PickupObject(GameWorld* world, const glm::vec3& position,
                           BaseModelInfo* modelinfo, PickupType type)
    : GameObject(world, position, glm::quat{1.0f, 0.0f, 0.0f, 0.0f}, modelinfo)
    , m_corona(world->createParticleEffect())
    , m_type(type) {
    btTransform tf;
    tf.setIdentity();
    tf.setOrigin(btVector3(position.x, position.y, position.z));

    m_ghost = std::make_unique<btPairCachingGhostObject>();
    m_ghost->setUserPointer(this);
    m_ghost->setWorldTransform(tf);
    m_shape = std::make_unique<btSphereShape>(0.5f);
    m_ghost->setCollisionShape(m_shape.get());
    m_ghost->setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT |
                               btCollisionObject::CF_NO_CONTACT_RESPONSE);

    m_colourId = 0;
    if (modelinfo->name == "colt45")
        m_colourId = 1;
    else if (modelinfo->name == "uzi")
        m_colourId = 2;
    else if (modelinfo->name == "shotgun")
        m_colourId = 3;
    else if (modelinfo->name == "ak47")
        m_colourId = 4;
    else if (modelinfo->name == "m16")
        m_colourId = 5;
    else if (modelinfo->name == "sniper" || modelinfo->name == "donkeymag")
        m_colourId = 6;
    else if (modelinfo->name == "rocket")
        m_colourId = 7;
    else if (modelinfo->name == "flame")
        m_colourId = 8;
    else if (modelinfo->name == "molotov")
        m_colourId = 9;
    else if (modelinfo->name == "grenade" || modelinfo->name == "package1" ||
             modelinfo->name == "floatpackge1")
        m_colourId = 10;
    else if (modelinfo->name == "bodyarmour" || modelinfo->name == "bribe")
        m_colourId = 11;
    else if (modelinfo->name == "info" || modelinfo->name == "killfrenzy")
        m_colourId = 12;
    else if (modelinfo->name == "health" || modelinfo->name == "bonus")
        m_colourId = 13;

    m_corona.position = getPosition();
    m_corona.direction = glm::vec3(0.f, 0.f, 1.f);
    m_corona.orientation = ParticleFX::Camera;

    // @todo float package should float on the water
    if (m_type == FloatingPackage) {
        // verify offset and texture?
        m_corona.position += glm::vec3(0.f, 0.f, 0.7f);
        m_corona.texture =
            engine->data->findSlotTexture("particle", "coronastar");
    } else {
        m_corona.texture =
            engine->data->findSlotTexture("particle", "coronaringa");
    }

    respawn = defaultDoesRespawn(m_type);
    respawnTime = defaultRespawnTime(m_type);
    behaviourFlags = defaultBehaviourFlags(m_type);

    setEnabled(true);
    setCollected(false);

    auto modelData = getModelInfo<SimpleModelInfo>();
    setModel(modelData->getAtomic(0)->clone(std::make_shared<ModelFrame>()));
    updateTransform(getPosition(), getRotation());
}

PickupObject::~PickupObject() {
    if (m_ghost) {
        setEnabled(false);
        engine->destroyEffect(m_corona);
    }
}

void PickupObject::tick(float dt) {
    if (isRampage()) {
        if (engine->state->scriptOnMissionFlag != nullptr) {
            if (*(engine->state->scriptOnMissionFlag) != 0 && isEnabled()) {
                setEnabled(false);
            } else if (*(engine->state->scriptOnMissionFlag) == 0 &&
                       !isEnabled()) {
                setEnabled(true);
            }
        }
    }

    if (!m_enabled) {
        // Check if our type of pickup respawns
        if (doesRespawn()) {
            m_enableTimer -= dt;
            if (m_enableTimer <= 0.f) {
                setEnabled(true);
                setCollected(false);
            }
        }
    }

    float time = engine->getGameTime();
    float colourValue = 0.5f * (std::sin(time * 3.0664064f) * 0.3f + 0.3f);
    uint32_t* colour = &colours[m_colourId];
    float red = static_cast<float>((*colour >> 16) & 0xFF);
    float green = static_cast<float>((*colour >> 8) & 0xFF);
    float blue = static_cast<float>(*colour & 0xFF);
    m_corona.colour =
        glm::vec4(red / 255.f, green / 255.f, blue / 255.f, 1.f) * colourValue;

    if (m_enabled) {
        static constexpr float kRotationSpeedCoeff = 3.0f;
        updateTransform(
            getPosition(),
            glm::angleAxis(engine->getGameTime() * kRotationSpeedCoeff,
                           glm::vec3(0.f, 0.f, 1.f)));

        // Sort out interactions with things that may or may not be players.
        btManifoldArray manifoldArray;
        btBroadphasePairArray& pairArray =
            m_ghost->getOverlappingPairCache()->getOverlappingPairArray();
        int numPairs = pairArray.size();
        auto flags = getBehaviourFlags();

        for (int i = 0; i < numPairs; i++) {
            manifoldArray.clear();

            const btBroadphasePair& pair = pairArray[i];
            auto otherObject = static_cast<const btCollisionObject*>(
                pair.m_pProxy0->m_clientObject == m_ghost.get()
                    ? pair.m_pProxy1->m_clientObject
                    : pair.m_pProxy0->m_clientObject);
            if (otherObject->getUserPointer()) {
                GameObject* object =
                    static_cast<GameObject*>(otherObject->getUserPointer());
                if ((flags & PickupOnFoot) == PickupOnFoot &&
                    object->type() == Character) {
                    CharacterObject* character =
                        static_cast<CharacterObject*>(object);

                    if (character->isPlayer()) {
                        setCollected(onPlayerTouch());
                        setEnabled(!isCollected());

                        if (!m_enabled) {
                            m_enableTimer = getRespawnTime();
                        }
                    }
                }
                if ((flags & PickupInVehicle) == PickupInVehicle &&
                    object->type() == Vehicle) {
                    VehicleObject* vehicle =
                        static_cast<VehicleObject*>(object);

                    if (vehicle->getOccupant(0) ==
                        static_cast<GameObject*>(
                            engine->getPlayer()->getCharacter())) {
                        setCollected(onPlayerVehicleTouch());
                        setEnabled(!isCollected());

                        if (!m_enabled) {
                            m_enableTimer = getRespawnTime();
                        }
                    }
                }
            }
        }
    }
}

void PickupObject::setEnabled(bool enabled) {
    if (!m_enabled && enabled) {
        engine->dynamicsWorld->addCollisionObject(
            m_ghost.get(), btBroadphaseProxy::SensorTrigger);
        m_corona.size = glm::vec2(1.5f, 1.5f);
    } else if (m_enabled && !enabled) {
        engine->dynamicsWorld->removeCollisionObject(m_ghost.get());
        m_corona.size = glm::vec2(0.f, 0.f);
    }

    m_enabled = enabled;
}

ItemPickup::ItemPickup(GameWorld* world, const glm::vec3& position,
                       BaseModelInfo* modelinfo, PickupType type,
                       const WeaponData& p_item)
    : PickupObject(world, position, modelinfo, type), item(p_item) {
}

bool ItemPickup::onPlayerTouch() {
    auto totalRounds = 0;

    switch (item.modelID) {
        case 173: /* Pistol */
            totalRounds = 45;
            break;
        case 178: /* Uzi */
            totalRounds = 125;
            break;
        case 176: /* Shotgun */
            totalRounds = 25;
            break;
        case 170: /* Grenade */
            totalRounds = 5;
            break;
        case 174: /* Molotov */
            totalRounds = 5;
            break;
        case 181: /* Flame thrower */
            totalRounds = 25;
            break;
        case 171: /* AK */
            totalRounds = 150;
            break;
        case 180: /* M16 */
            totalRounds = 300;
            break;
        case 177: /* Sniper Rifle */
            totalRounds = 25;
            break;
    }

    if (getPickupType() == OnStreet || getPickupType() == OnStreetSlow) {
        totalRounds /= 5;
    }

    auto character = engine->getPlayer()->getCharacter();

    character->addToInventory(item.inventorySlot, totalRounds);

    return true;
}

DummyPickup::DummyPickup(GameWorld* world, const glm::vec3& position,
                         BaseModelInfo* modelinfo, PickupType type)
    : PickupObject(world, position, modelinfo, type) {
}

bool DummyPickup::onPlayerTouch() {
    return true;
}

RampagePickup::RampagePickup(GameWorld* world, const glm::vec3& position,
                             BaseModelInfo* modelinfo, PickupType type)
    : PickupObject(world, position, modelinfo, type) {
}

bool RampagePickup::onPlayerTouch() {
    if (engine->state->scriptOnMissionFlag == nullptr) {
        return false;
    }

    if (*(engine->state->scriptOnMissionFlag) != 0) {
        return false;
    }

    return true;
}

HealthPickup::HealthPickup(GameWorld* world, const glm::vec3& position,
                           BaseModelInfo* modelinfo, PickupType type)
    : PickupObject(world, position, modelinfo, type) {
}

bool HealthPickup::onPlayerTouch() {
    auto character = engine->getPlayer()->getCharacter();

    if (character->getCurrentState().health >= 100.f) {
        return false;
    }

    character->getCurrentState().health = 100.f;

    return true;
}

ArmourPickup::ArmourPickup(GameWorld* world, const glm::vec3& position,
                           BaseModelInfo* modelinfo, PickupType type)
    : PickupObject(world, position, modelinfo, type) {
}

bool ArmourPickup::onPlayerTouch() {
    auto character = engine->getPlayer()->getCharacter();

    if (character->getCurrentState().armour >= 100.f) {
        return false;
    }

    character->getCurrentState().armour = 100.f;

    return true;
}

CollectablePickup::CollectablePickup(GameWorld* world,
                                     const glm::vec3& position,
                                     BaseModelInfo* modelinfo, PickupType type)
    : PickupObject(world, position, modelinfo, type) {
}

bool CollectablePickup::onPlayerTouch() {
    auto state = engine->state;

    if (state->playerInfo.hiddenPackagesCollected ==
        state->playerInfo.hiddenPackageCount) {
        state->playerInfo.money += 1000000;

        const auto gxtEntry = "CO_ALL";

        state->text.addText<ScreenTextType::HiddenPackageText>(
            ScreenTextEntry::makeHiddenPackageText(
                gxtEntry, engine->data->texts.text(gxtEntry)));

    } else {
        state->playerInfo.hiddenPackagesCollected++;
        state->playerInfo.money += 1000;

        const auto gxtEntry = "CO_ONE";

        auto text = ScreenText::format(
            engine->data->texts.text(gxtEntry),
            GameStringUtil::fromString(
                std::to_string(state->playerInfo.hiddenPackagesCollected),
                FONT_PRICEDOWN),
            GameStringUtil::fromString(
                std::to_string(state->playerInfo.hiddenPackageCount),
                FONT_PRICEDOWN));

        state->text.addText<ScreenTextType::HiddenPackageText>(
            ScreenTextEntry::makeHiddenPackageText(gxtEntry, text));
    }

    return true;
}

AdrenalinePickup::AdrenalinePickup(GameWorld* world, const glm::vec3& position,
                                   BaseModelInfo* modelinfo, PickupType type)
    : PickupObject(world, position, modelinfo, type) {
}

bool AdrenalinePickup::onPlayerTouch() {
    engine->getPlayer()->activateAdrenalineEffect();

    return true;
}

MoneyPickup::MoneyPickup(GameWorld* world, const glm::vec3& position,
                         BaseModelInfo* modelinfo, PickupType type,
                         uint32_t money)
    : PickupObject(world, position, modelinfo, type), money(money) {
}

bool MoneyPickup::onPlayerTouch() {
    engine->state->playerInfo.money += money;

    return true;
}

BigNVeinyPickup::BigNVeinyPickup(GameWorld* world, const glm::vec3& position,
                                 BaseModelInfo* modelinfo, PickupType type)
    : PickupObject(world, position, modelinfo, type) {
}

bool BigNVeinyPickup::onPlayerVehicleTouch() {
    engine->state->bigNVeinyPickupsCollected++;

    return true;
}

const std::array<glm::vec3, 106>&
BigNVeinyPickup::getBigNVeinyPickupsLocations() {
    return kBigNVeinyPickupsLocations;
}
