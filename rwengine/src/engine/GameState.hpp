#ifndef _RWENGINE_GAMESTATE_HPP_
#define _RWENGINE_GAMESTATE_HPP_
#include <bitset>
#include <cstdint>
#include <string>
#include <map>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <data/GameTexts.hpp>
#include <data/VehicleGenerator.hpp>
#include <engine/GameInputState.hpp>
#include <engine/ScreenText.hpp>
#include <objects/ObjectTypes.hpp>

class GameWorld;
class GameObject;
class ScriptMachine;
struct CutsceneData;

struct SystemTime {
    uint16_t year;
    uint16_t month;
    uint16_t dayOfWeek;
    uint16_t day;
    uint16_t hour;
    uint16_t minute;
    uint16_t second;
    uint16_t millisecond;
};

/** Block 0 State */
struct BasicState {
    GameStringChar saveName[24];
    SystemTime saveTime;
    uint32_t unknown;
    uint16_t islandNumber;
    glm::vec3 cameraPosition;
    uint32_t gameMinuteMS;
    uint32_t lastTick;
    uint8_t gameHour;
    uint8_t _align0[3];
    uint8_t gameMinute;
    uint8_t _align1[3];
    uint16_t padMode;
    uint8_t _align2[2];
    uint32_t timeMS;
    float timeScale;
    float timeStep;
    float timeStep_unclipped;  // Unknown purpose
    uint32_t frameCounter;
    float timeStep2;
    float framesPerUpdate;
    float timeScale2;
    uint16_t lastWeather;
    uint8_t _align3[2];
    uint16_t nextWeather;
    uint8_t _align4[2];
    uint16_t forcedWeather;
    uint8_t _align5[2];
    float weatherInterpolation;
    uint8_t dateTime[24];  // Unused
    uint32_t weatherType;
    float cameraData;
    float cameraData2;

    BasicState();
};

/** Block 16 player info */
struct PlayerInfo {
    uint32_t money;
    uint8_t unknown1;
    uint32_t unknown2;
    uint16_t unknown3;
    float unknown4;
    uint32_t displayedMoney;
    uint32_t hiddenPackagesCollected;
    uint32_t hiddenPackageCount;
    uint8_t neverTired;
    uint8_t fastReload;
    uint8_t thaneOfLibertyCity;
    uint8_t singlePayerHealthcare;
    uint8_t unknown5[70];

    PlayerInfo();
};

/** Block 17 */
struct GameStats {
    uint32_t playerKills;
    uint32_t otherKills;
    uint32_t carsExploded;
    uint32_t shotsHit;
    uint32_t pedTypesKilled[23];
    uint32_t helicoptersDestroyed;
    uint32_t playerProgress;
    uint32_t explosiveKgsUsed;
    uint32_t bulletsFired;
    uint32_t bulletsHit;
    uint32_t carsCrushed;
    uint32_t headshots;
    uint32_t timesBusted;
    uint32_t timesHospital;
    uint32_t daysPassed;
    uint32_t mmRainfall;
    uint32_t insaneJumpMaxDistance;
    uint32_t insaneJumpMaxHeight;
    uint32_t insaneJumpMaxFlips;
    uint32_t insangeJumpMaxRotation;
    /*
     * 0 none completed
     * 1 insane stunt
     * 2 perfect insane stunt
     * 3 double insane stunt
     * 4 perfect double insane stunt
     * 5 triple insane stunt
     * 6 perfect " " "
     * 7 quadruple
     * 8 perfect quadruple
     */
    uint32_t bestStunt;
    uint32_t uniqueStuntsFound;
    uint32_t uniqueStuntsTotal;
    uint32_t missionAttempts;
    uint32_t missionsPassed;
    uint32_t passengersDroppedOff;
    uint32_t taxiRevenue;
    uint32_t portlandPassed;
    uint32_t stauntonPassed;
    uint32_t shoresidePassed;
    uint32_t bestTurismoTime;
    float distanceWalked;
    float distanceDriven;
    uint32_t patriotPlaygroundTime;
    uint32_t aRideInTheParkTime;
    uint32_t grippedTime;
    uint32_t multistoryMayhemTime;
    uint32_t peopleSaved;
    uint32_t criminalsKilled;
    uint32_t highestParamedicLevel;
    uint32_t firesExtinguished;
    uint32_t longestDodoFlight;
    uint32_t bombDefusalTime;
    uint32_t rampagesPassed;
    uint32_t totalRampages;
    uint32_t totalMissions;
    uint32_t fastestTime[16];  // not used
    uint32_t highestScore[16];
    uint32_t peopleKilledSinceCheckpoint;  // ?
    uint32_t peopleKilledSinceLastBustedOrWasted;
    char lastMissionGXT[8];

    GameStats();
};

struct TextDisplayData {
    // This is set by the final display text command.
    GameString text;
    glm::vec2 position;

    glm::vec4 colourFG;
    glm::vec4 colourBG;
};

/**
 * Data about a blip
 */
struct BlipData {
    int id;

    enum BlipType {
        Location = 0,
        Vehicle = 1,
        Pickup = 2,
        Character = 3,
        Instance = 4,
    };
    BlipType type;
    GameObjectID target;
    // If target is null then use coord
    glm::vec3 coord;

    std::string texture;  // Texture for use in the radar
    uint32_t colour = 0;  // Color value (index or RGBA)
    bool dimmed = false;  // Color dimming if not in RGBA mode

    uint16_t size = 3;  // Only used if texture is empty

    enum DisplayMode { Hide = 0, MarkerOnly = 1, RadarOnly = 2, ShowBoth = 3 };

    /* Should the blip be displayed? */
    DisplayMode display;

    BlipData() : id(-1), type(Location), target(0), display(ShowBoth) {
    }

    int getScriptObjectID() const {
        return id;
    }
};

/**
 * Data for garages
 */
struct GarageInfo {
    enum /*GarageType*/ {
        GARAGE_MISSION = 1,
        GARAGE_BOMBSHOP1 = 2,
        GARAGE_BOMBSHOP2 = 3,
        GARAGE_BOMBSHOP3 = 4,
        GARAGE_RESPRAY = 5,
        GARAGE_INVALID = 6,
        GARAGE_SPECIFIC_CARS_ONLY = 7, /* See Opcode 0x21B */
        GARAGE_COLLECTCARS1 = 8,       /* See Opcode 0x03D4 */
        GARAGE_COLLECTCARS2 = 9,
        GARAGE_COLLECTCARS3 = 10, /* Unused */
        GARAGE_OPENFOREXIT = 11,
        GARAGE_INVALID2 = 12,
        GARAGE_CRUSHER = 13,
        GARAGE_MISSION_KEEPCAR = 14,
        GARAGE_FOR_SCRIPT = 15,
        GARAGE_HIDEOUT_ONE = 16,   /* Portland */
        GARAGE_HIDEOUT_TWO = 17,   /* Staunton */
        GARAGE_HIDEOUT_THREE = 18, /* Shoreside */
        GARAGE_FOR_SCRIPT2 = 19,
        GARAGE_OPENS_FOR_SPECIFIC_CAR = 20,
        GARAGE_OPENS_ONCE = 21
    };
    int id;
    glm::vec3 min;
    glm::vec3 max;
    int type;

    GarageInfo(int id_, const glm::vec3 min_, const glm::vec3 max_, int type_)
        : id(id_), min(min_), max(max_), type(type_) {
    }

    int getScriptObjectID() const {
        return id;
    }
};

/**
 * Gameplay state object that holds persistent state, and references runtime
 * world state.
 */
class GameState {
public:
    /**
      Basic Game State
     */
    BasicState basic;

    /**
      Player stats
     */
    PlayerInfo playerInfo;

    /**
      Game Stats
      */
    GameStats gameStats;

    /**
     * Second since game was started
     */
    float gameTime;
    unsigned int currentProgress;
    unsigned int maxProgress;

    unsigned int maxWantedLevel;

    GameObjectID playerObject;

    /**
     * @brief Stores a pointer to script global that stores the on-mission
     * state.
     */
    int32_t* scriptOnMissionFlag;

    /** Objects created by the current mission */
    std::vector<GameObject*> missionObjects;

    bool overrideNextStart;
    glm::vec4 nextRestartLocation;

    bool fadeOut;
    float fadeStart;
    float fadeTime;
    bool fadeSound;
    glm::u16vec3 fadeColour;

    std::string currentSplash;

    bool skipCutscene;
    bool isIntroPlaying;
    CutsceneData* currentCutscene;
    float cutsceneStartTime;
    /** Flag for rendering cutscene letterbox */
    bool isCinematic;

    std::string lastMissionName;

    /// Stores the "special" character and cutscene model indices.
    std::map<unsigned short, std::string> specialCharacters;
    std::map<unsigned short, std::string> specialModels;

    /// Handles on screen text behaviour
    ScreenText text;

    TextDisplayData nextText;
    /**
     * Stores temporary, one-tick messages
     */
    std::vector<TextDisplayData> texts;

    /** The camera near value currently set by the script */
    float cameraNear;
    bool cameraFixed;
    glm::vec3 cameraPosition;
    glm::quat cameraRotation;

    GameObjectID cameraTarget;

    std::vector<VehicleGenerator> vehicleGenerators;

    std::map<int, BlipData> radarBlips;

    std::vector<GarageInfo> garages;

    /**
     * Bitsets for the car import / export list mission
     */
    std::bitset<32> importExportPortland;
    std::bitset<32> importExportShoreside;
    std::bitset<32> importExportUnused;

    /**
     * State of the game input for the last 2 frames
     */
    GameInputState input[2];

    /**
     * World to use for this state, this isn't saved, just used at runtime
     */
    GameWorld* world;

    /**
     * Script Machine associated with this state if it exists.
     */
    ScriptMachine* script;

    GameState();

    /**
     * Adds a blip to the state, returning it's ID.
     */
    int addRadarBlip(BlipData& blip);

    /**
     * Removes a blip
     */
    void removeBlip(int blip);

    /**
     * Swaps input state
     */
    void swapInputState() {
        input[1] = input[0];
    }
};

#endif
