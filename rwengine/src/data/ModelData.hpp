#ifndef _RWENGINE_MODELDATA_HPP_
#define _RWENGINE_MODELDATA_HPP_
#include <array>
#include <cstdint>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <data/Clump.hpp>
#include <rw/defines.hpp>
#include <rw/forward.hpp>

#include <data/CollisionModel.hpp>
#include <data/PathData.hpp>
#ifdef RW_WINDOWS
#include <rw_mingw.hpp>
#endif

/**
 * 16-bit model ID identifier (from .ide)
 */
using ModelID = uint16_t;

enum class ModelDataType {
    SimpleInfo = 1,
    /** Unknown */
    MLoModelInfo = 2,
    /** Currently unused; data in SimpleInfo instead */
    TimeModelInfo = 3,
    ClumpInfo = 4,
    VehicleInfo = 5,
    PedInfo = 6
};

/**
 * Base type for all model information
 *
 * @todo reference counting
 * @todo store collision model
 */
class BaseModelInfo {
public:
    std::string name;
    std::string textureslot;

    BaseModelInfo(ModelDataType type) : type_(type) {
    }

    virtual ~BaseModelInfo() = default;

    ModelID id() const {
        return modelid_;
    }

    void setModelID(ModelID id) {
        modelid_ = id;
    }

    ModelDataType type() const {
        return type_;
    }

    void addReference() {
        refcount_++;
    }

    void removeReference() {
        refcount_--;
    }

    int getReferenceCount() const {
        return refcount_;
    }

    void setCollisionModel(std::unique_ptr<CollisionModel>& col) {
        collision = std::move(col);
    }

    CollisionModel* getCollision() const {
        return collision.get();
    }

    /// @todo replace with proper streaming implementation
    virtual bool isLoaded() const = 0;

    virtual void unload() = 0;

    static std::string getTypeName(ModelDataType type) {
        switch (type) {
            case ModelDataType::SimpleInfo:
                return "Simple";
            case ModelDataType::VehicleInfo:
                return "Vehicle";
            case ModelDataType::PedInfo:
                return "Pedestrian";
            case ModelDataType::ClumpInfo:
                return "Cutscene";
            default:
                break;
        }
        return "Unknown";
    }

private:
    ModelID modelid_ = 0;
    ModelDataType type_;
    int refcount_ = 0;
    std::unique_ptr<CollisionModel> collision;
};

using ModelInfoTable =
    std::unordered_map<ModelID, std::unique_ptr<BaseModelInfo>>;

static std::unordered_set<std::string> doorModels = {
    "oddjgaragdoor",      "bombdoor",           "door_bombshop",
    "vheistlocdoor",      "door2_garage",       "ind_slidedoor",
    "bankjobdoor",        "door_jmsgrage",      "jamesgrge_kb",
    "door_sfehousegrge",  "shedgaragedoor",     "door4_garage",
    "door_col_compnd_01", "door_col_compnd_02", "door_col_compnd_03",
    "door_col_compnd_04", "door_col_compnd_05", "impex_door",
    "SalvGarage",         "door3_garage",       "leveldoor2",
    "double_garage_dr",   "amcogaragedoor",     "towergaragedoor1",
    "towergaragedoor2",   "towergaragedoor3",   "plysve_gragedoor",
    "impexpsubgrgdoor",   "Sub_sprayshopdoor",  "ind_plyrwoor",
    "8ballsuburbandoor",  "crushercrush",       "crushertop",
};

/**
 * Model data for simple types
 *
 * @todo replace Model* with librw types
 */
class SimpleModelInfo : public BaseModelInfo {
public:
    static constexpr ModelDataType kType = ModelDataType::SimpleInfo;

    /// @todo Use TimeModelInfo instead of hacking this in here
    int timeOn = 0;
    int timeOff = 24;
    int flags;
    /// Information loaded from PATH sections
    /// @todo remove this from here too :)
    std::vector<PathData> paths;

    SimpleModelInfo() : BaseModelInfo(kType) {
    }
    SimpleModelInfo(ModelDataType type) : BaseModelInfo(type) {
    }

    /// @todo change with librw
    void setAtomic(const ClumpPtr& model, int n, const AtomicPtr& atomic) {
        model_ = model;
        /// @todo disassociated the Atomic from Clump
        atomics_[n] = atomic;
    }

    /// @todo remove this
    ClumpPtr getModel() const {
        return model_;
    }

    Atomic* getAtomic(int n) const {
        return atomics_[n].get();
    }

    void setLodDistance(int n, float d) {
        RW_CHECK(n < 3, "Lod Index out of range");
        loddistances_[n] = d;
    }

    float getLodDistance(int n) {
        RW_CHECK(n < 3, "Lod Index out of range");
        return loddistances_[n];
    }

    Atomic* getDistanceAtomic(float d) {
        for (auto i = 0; i < getNumAtomics(); ++i) {
            if (d < loddistances_[i]) {
                return atomics_[i].get();
            }
        }
        return nullptr;
    }

    void setNumAtomics(int num) {
        numatomics_ = num;
    }

    int getNumAtomics() const {
        return numatomics_;
    }

    bool isLoaded() const override {
        return model_.get() != nullptr;
    }

    void unload() override {
        model_ = nullptr;
    }

    enum {
        /// Cull model if player doesn't look at it. Ignored in GTA 3.
        NORMAL_CULL = 1,
        /// Do not fade the object when it is being
        /// loaded into or out of view.
        DO_NOT_FADE = 1 << 1,
        /// Model is transparent. Render this object after
        /// all opaque objects, allowing transparencies of
        /// other objects to be visible through this
        /// object.
        DRAW_LAST = 1 << 2,
        /// Render with additive blending. Previous flag
        /// must be enabled too.
        ADDITIVE = 1 << 3,
        /// Model is a tunnel, i.e. set the object as
        /// invisible unless the player enters cull zone
        /// flag 128. This flag works only with static
        /// models.
        IS_SUBWAY = 1 << 4,
        /// Don't use static lighting, we want
        /// dynamic if it's possible.
        IGNORE_LIGHTING = 1 << 5,
        /// Model is a shadow. Disable writing to z-buffer when
        /// rendering it, allowing transparencies of other objects,
        /// shadows, and lights to be visible through this object.
        /// (Not implemented in the PS2 version)
        NO_ZBUFFER_WRITE = 1 << 6,
    };

    // Set up data for big building objects
    void setupBigBuilding(const ModelInfoTable& models);
    bool isBigBuilding() const {
        return isbigbuilding_;
    }

    void findRelatedModel(const ModelInfoTable& models);

    float getLargestLodDistance() const {
        return furthest_ != 0 ? loddistances_[furthest_ - 1]
                              : loddistances_[numatomics_ - 1];
    }

    float getNearLodDistance() const {
        return loddistances_[2];
    }

    void determineFurthest() {
        furthest_ = 0;
        if (numatomics_ == 2) {
            furthest_ = loddistances_[0] >= loddistances_[1] ? 1 : 0;
        }
        if (numatomics_ == 3) {
            furthest_ = loddistances_[0] >= loddistances_[1]
                            ? 1
                            : loddistances_[1] >= loddistances_[2] ? 2 : 0;
        }
    }

    SimpleModelInfo* related() const {
        return related_;
    }

    static bool isDoorModel(std::string m) {
        return doorModels.find(m) != doorModels.end();
    }

private:
    ClumpPtr model_;
    std::array<AtomicPtr, 3> atomics_;
    float loddistances_[3] = {};
    uint8_t numatomics_ = 0;
    uint8_t alpha_ = 0;  /// @todo ask aap why
    bool isbigbuilding_ = 0;
    uint8_t furthest_ = 0;

    SimpleModelInfo* related_ = nullptr;
};

/**
 * @todo this
 */
class TimeModelInfo : public SimpleModelInfo {
    TimeModelInfo() : SimpleModelInfo(ModelDataType::TimeModelInfo) {
    }
};

/**
 * @todo document me
 */
class ClumpModelInfo : public BaseModelInfo {
public:
    static constexpr ModelDataType kType = ModelDataType::ClumpInfo;

    ClumpModelInfo() : BaseModelInfo(kType) {
    }
    ClumpModelInfo(ModelDataType type) : BaseModelInfo(type) {
    }

    void setModel(const ClumpPtr& model) {
        model_ = model;
    }

    ClumpPtr getModel() const {
        return model_;
    }

    bool isLoaded() const override {
        return model_.get() != nullptr;
    }

    void unload() override {
        model_ = nullptr;
    }

private:
    ClumpPtr model_ = nullptr;
};

enum class ComponentRuleType {
    Any = 1,
    RainOnly = 2,
    Optional = 3,
    Random = 4
};

/**
 * Data for a vehicle model type
 */
class VehicleModelInfo : public ClumpModelInfo {
public:
    static constexpr ModelDataType kType = ModelDataType::VehicleInfo;

    VehicleModelInfo() : ClumpModelInfo(kType) {
    }

    enum VehicleClass {
        IGNORE = 0,
        NORMAL = 1,
        POORFAMILY = 1 << 1,
        RICHFAMILY = 1 << 2,
        EXECUTIVE = 1 << 3,
        WORKER = 1 << 4,
        BIG = 1 << 5,
        TAXI = 1 << 6,
        MOPED = 1 << 7,
        MOTORBIKE = 1 << 8,
        LEISUREBOAT = 1 << 9,
        WORKERBOAT = 1 << 10,
        BICYCLE = 1 << 11,
        ONFOOT = 1 << 12,
        /// @todo verify that this is the correct bit
        SPECIAL = 1 << 13,
    };

    enum VehicleType {
        CAR,
        BOAT,
        TRAIN,
        PLANE,
        HELI,
    };

    VehicleType vehicletype_;
    ModelID wheelmodel_;
    float wheelscale_;
    int numdoors_;
    std::string handling_;
    VehicleClass vehicleclass_;
    int frequency_;
    int level_;
    unsigned long componentrules_;
    std::string vehiclename_;

    static VehicleType findVehicleType(const std::string& name) {
        static const std::unordered_map<std::string, VehicleType> vehicleTypes{
            {"car", CAR},
            {"boat", BOAT},
            {"train", TRAIN},
            {"plane", PLANE},
            {"heli", HELI}};
        return vehicleTypes.at(name);
    }

    static VehicleClass findVehicleClass(const std::string& name) {
        static const std::unordered_map<std::string, VehicleClass> classTypes{
            // III, VC, SA
            {"ignore", IGNORE},
            {"normal", NORMAL},
            {"poorfamily", POORFAMILY},
            {"richfamily", RICHFAMILY},
            {"executive", EXECUTIVE},
            {"worker", WORKER},
            {"big", BIG},
            {"taxi", TAXI},
            {"special", SPECIAL},
            // VC, SA
            {"moped", MOPED},
            {"motorbike", MOTORBIKE},
            {"leisureboat", LEISUREBOAT},
            {"workerboat", WORKERBOAT},
            {"bicycle", BICYCLE},
            {"onfoot", ONFOOT},
        };
        return classTypes.at(name);
    }
};

class PedModelInfo : public ClumpModelInfo {
public:
    static constexpr ModelDataType kType = ModelDataType::PedInfo;

    PedModelInfo() : ClumpModelInfo(kType) {
    }

    enum PedType {
        // III
        PLAYER1 = 0,
        PLAYER2,
        PLAYER3,
        PLAYER4,
        CIVMALE,
        CIVFEMALE,
        COP,
        GANG1,
        GANG2,
        GANG3,
        GANG4,
        GANG5,
        GANG6,
        GANG7,
        GANG8,
        GANG9,
        EMERGENCY,
        FIREMAN,
        CRIMINAL,
        _UNNAMED,
        PROSTITUTE,
        SPECIAL,
        _NUM_PEDTYPE
    };

    PedType pedtype_ = PLAYER1;
    int statindex_ = 0;
    /// @todo this should be an index
    std::string animgroup_;
    /// The mask of vehicle classes this ped can drive
    int carsmask_ = 0;

    static PedType findPedType(const std::string& name) {
        static const std::unordered_map<std::string, PedType> pedTypes{
            // III
            {"PLAYER1", PLAYER1},
            {"PLAYER2", PLAYER2},
            {"PLAYER3", PLAYER3},
            {"PLAYER4", PLAYER4},
            {"CIVMALE", CIVMALE},
            {"CIVFEMALE", CIVFEMALE},
            {"COP", COP},
            {"GANG1", GANG1},
            {"GANG2", GANG2},
            {"GANG3", GANG3},
            {"GANG4", GANG4},
            {"GANG5", GANG5},
            {"GANG6", GANG6},
            {"GANG7", GANG7},
            {"GANG8", GANG8},
            {"GANG9", GANG9},
            {"EMERGENCY", EMERGENCY},
            {"FIREMAN", FIREMAN},
            {"CRIMINAL", CRIMINAL},
            {"PROSTITUTE", PROSTITUTE},
            {"SPECIAL", SPECIAL},
        };
        return pedTypes.at(name);
    }
};

/**
 * This is orthogonal to object class, it gives
 * Instances different physical properties.
 */
struct DynamicObjectData {
    std::string modelName;
    float mass;        // Kg
    float turnMass;    // Kg m^3
    float airRes;      // fraction
    float elasticity;  // "
    float buoyancy;
    float uprootForce;  // Force
    float collDamageMulti;

    enum {
        Damage_ChangeModel = 1,
        Damage_SplitModel = 2,
        Damage_Smash = 3,
        Damage_ChangeThenSmash = 4,
        Damage_SmashCardboard = 50,
        Damage_SmashWoodenBox = 60,
        Damage_SmashTrafficCone = 70,
        Damage_SmashBarPost = 80,
    };
    uint8_t collDamageEffect;

    enum {
        Response_None = 0,
        Response_LampPost = 1,
        Response_SmallBox = 2,
        Response_BigBox = 3,
        Response_FencePart = 4,
    };
    uint8_t collResponseFlags;

    bool cameraAvoid;
};

#endif
