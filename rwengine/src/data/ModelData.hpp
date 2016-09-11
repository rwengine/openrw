#ifndef RWENGINE_MODELDATA_HPP
#define RWENGINE_MODELDATA_HPP
#include <stdint.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <data/Model.hpp>
#include <data/PathData.hpp>
#include <rw/defines.hpp>
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

    virtual ~BaseModelInfo() {
    }

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

    /// @todo replace with proper streaming implementation
    virtual bool isLoaded() const = 0;

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
    /// @todo Remove this?
    bool LOD = false;
    /// Information loaded from PATH sections
    /// @todo remove this from here too :)
    std::vector<PathData> paths;

    SimpleModelInfo() : BaseModelInfo(kType) {
    }
    SimpleModelInfo(ModelDataType type) : BaseModelInfo(type) {
    }

    /// @todo change with librw
    void setAtomic(Model* model, int n, ModelFrame* atomic) {
        model_ = model;
        atomics_[n] = atomic;
    }

    ModelFrame* getAtomic(int n) const {
        return atomics_[n];
    }

    void setLodDistance(int n, float d) {
        RW_CHECK(n < 3, "Lod Index out of range");
        loddistances_[n] = d;
    }

    float getLodDistance(int n) {
        RW_CHECK(n < 3, "Lod Index out of range");
        return loddistances_[n];
    }

    void setNumAtomics(int num) {
        numatomics_ = num;
    }

    int getNumAtomics() const {
        return numatomics_;
    }

    bool isLoaded() const override {
        return model_ != nullptr;
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

private:
    Model* model_ = nullptr;
    ModelFrame* atomics_[3] = {};
    float loddistances_[3] = {};
    uint8_t numatomics_ = 0;
    uint8_t alpha_ = 0;  /// @todo ask aap why
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

    void setModel(Model* model) {
        model_ = model;
    }

    Model* getModel() const {
        return model_;
    }

    bool isLoaded() const override {
        return model_ != nullptr;
    }

private:
    Model* model_ = nullptr;
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
    int componentrules_;
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
        PLAYER_4,
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
    };

    PedType pedtype_ = PLAYER1;
    /// @todo this should be an index
    std::string behaviour_;
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
            {"PLAYER_4", PLAYER_4},
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
            {"_UNNAMED", _UNNAMED},
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
    float elacticity;  // "
    float bouancy;
    float uprootForce;  // Force
    float collDamageMulti;
    /*
     * 1: change model
     * 2: split model
     * 3: smash
     * 4: change and smash
     */
    uint8_t collDamageFlags;
    /*
     * 1: lampost
     * 2: smallbox
     * 3: bigbox
     * 4: fencepart
     */
    uint8_t collResponseFlags;
    bool cameraAvoid;
};

#endif
