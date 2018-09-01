#ifndef _RWENGINE_PICKUPOBJECT_HPP_
#define _RWENGINE_PICKUPOBJECT_HPP_

#include <cstdint>
#include <glm/glm.hpp>

#include <rw/debug.hpp>

#include <render/VisualFX.hpp>
#include <objects/GameObject.hpp>


class btPairCachingGhostObject;
class btSphereShape;

class BaseModelInfo;
class GameWorld;
class CharacterObject;
class VehicleObject;
struct VisualFX;

/**
 * @brief The PickupObject class
 * Implements interface and base behaviour for pickups
 */
class PickupObject : public GameObject {
public:
    enum PickupType {
        InShop = 1,
        OnStreet = 2,
        Once = 3,
        OnceTimeout = 4,
        Collectable = 5,
        OutOfStock = 6,
        Money = 7,
        MineInactive = 8,
        MineArmed = 9,
        NauticalMineInactive = 10,
        NauticalMineArmed = 11,
        FloatingPackage = 12,
        FloatingPackageFloating = 13,
        OnStreetSlow = 14
    };
    enum BehaviourFlags { None = 0, PickupOnFoot = 1, PickupInVehicle = 2 };

    static bool defaultDoesRespawn(PickupType type);
    static float defaultRespawnTime(PickupType type);
    static BehaviourFlags defaultBehaviourFlags(PickupType type);

    PickupObject(GameWorld* world, const glm::vec3& position,
                 BaseModelInfo* modelinfo, PickupType type);

    ~PickupObject() override;

    Type type() const override {
        return Pickup;
    }

    void tick(float dt) override;

    virtual bool onPlayerTouch() {
        return false;
    }

    virtual bool onPlayerVehicleTouch() {
        return false;
    }

    bool isEnabled() const {
        return m_enabled;
    }
    void setEnabled(bool enabled);

    bool isCollected() const {
        return m_collected;
    }

    void setCollected(bool collected) {
        m_collected = collected;
    }

    PickupType getPickupType() const {
        return m_type;
    }

    virtual bool isRampage() const {
        return false;
    }

    virtual bool isBigNVeinyPickup() const {
        return false;
    }

    bool doesRespawn() const {
        return respawn;
    }

    void setRespawn(bool r) {
        respawn = r;
    }

    float getRespawnTime() const {
        return respawnTime;
    }

    void setRespawnTime(float time) {
        respawnTime = time;
    }

    BehaviourFlags getBehaviourFlags() const {
        return behaviourFlags;
    }

    void setBehaviourFlags(BehaviourFlags flags) {
        behaviourFlags = flags;
    }

private:
    std::unique_ptr<btPairCachingGhostObject> m_ghost;
    std::unique_ptr<btSphereShape> m_shape;
    bool m_enabled = false;
    float m_enableTimer = 0.f;
    bool m_collected = false;
    ParticleFX& m_corona;
    short m_colourId = 0;
    bool respawn = false;
    float respawnTime{};
    BehaviourFlags behaviourFlags{};

    PickupType m_type{};
};

/**
 * @brief The ItemPickup class
 * Inserts an item into a characters inventory on pickup.
 */

struct WeaponData;

class ItemPickup : public PickupObject {
    WeaponData* item;

public:
    ItemPickup(GameWorld* world, const glm::vec3& position,
               BaseModelInfo* modelinfo, PickupType type, WeaponData* item);

    bool onPlayerTouch() override;
};

/**
 * @brief The DummyPickup class
 */
class DummyPickup : public PickupObject {
public:
    DummyPickup(GameWorld* world, const glm::vec3& position,
                BaseModelInfo* modelinfo, PickupType type);

    bool onPlayerTouch() override;
};

/**
 * @brief The RampagePickup class
 */
class RampagePickup : public PickupObject {
public:
    RampagePickup(GameWorld* world, const glm::vec3& position,
                  BaseModelInfo* modelinfo, PickupType type);

    bool isRampage() const override {
        return true;
    }

    bool onPlayerTouch() override;
};

/**
 * @brief The HealthPickup class
 */
class HealthPickup : public PickupObject {
public:
    HealthPickup(GameWorld* world, const glm::vec3& position,
                 BaseModelInfo* modelinfo, PickupType type);

    bool onPlayerTouch() override;
};

/**
 * @brief The ArmourPickup class
 */
class ArmourPickup : public PickupObject {
public:
    ArmourPickup(GameWorld* world, const glm::vec3& position,
                 BaseModelInfo* modelinfo, PickupType type);

    bool onPlayerTouch() override;
};

/**
 * @brief The CollectablePickup class
 */
class CollectablePickup : public PickupObject {
public:
    CollectablePickup(GameWorld* world, const glm::vec3& position,
                      BaseModelInfo* modelinfo, PickupType type);

    bool onPlayerTouch() override;
};

/**
 * @brief The AdrenalinePickup class
 */
class AdrenalinePickup : public PickupObject {
public:
    AdrenalinePickup(GameWorld* world, const glm::vec3& position,
                     BaseModelInfo* modelinfo, PickupType type);

    bool onPlayerTouch() override;
};

/**
 * @brief The MoneyPickup class
 */
class MoneyPickup : public PickupObject {
    uint32_t money;

public:
    MoneyPickup(GameWorld* world, const glm::vec3& position,
                BaseModelInfo* modelinfo, PickupType type, uint32_t money);

    void setMoney(uint32_t m) {
        money = m;
    };

    bool onPlayerTouch() override;
};

const static std::array<glm::vec3, 106> bigNVeinyPickupsLocations = {{
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

/**
 * @brief The BigNVeinyPickup class
 */
class BigNVeinyPickup : public PickupObject {
public:
    BigNVeinyPickup(GameWorld* world, const glm::vec3& position,
                    BaseModelInfo* modelinfo, PickupType type);

    bool onPlayerVehicleTouch() override;

    bool isBigNVeinyPickup() const override {
        return true;
    }

    const static std::array<glm::vec3, 106>& getBigNVeinyPickupsLocations() {
        return bigNVeinyPickupsLocations;
    }
};

#endif
