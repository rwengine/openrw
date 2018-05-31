#ifndef _RWENGINE_GAMESTATE_HPP_
#define _RWENGINE_GAMESTATE_HPP_
#include <bitset>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <data/GameTexts.hpp>
#include <data/VehicleGenerator.hpp>
#include <engine/GameData.hpp>
#include <engine/GameInputState.hpp>

#include <engine/GameWorld.hpp>
#include <engine/ScreenText.hpp>
#include <objects/ObjectTypes.hpp>

#include <script/ScriptTypes.hpp>

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
    int32_t money;
    uint8_t unknown1;
    uint32_t unknown2;
    uint16_t unknown3;
    float unknown4;
    int32_t displayedMoney;
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
    int32_t insaneJumpMaxFlips;
    int32_t insaneJumpMaxRotation;
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
    int32_t bestStunt;
    uint32_t uniqueStuntsFound;
    uint32_t uniqueStuntsTotal;
    uint32_t missionAttempts;
    uint32_t missionsPassed;
    uint32_t passengersDroppedOff;
    uint32_t taxiRevenue;
    uint32_t portlandPassed;
    uint32_t stauntonPassed;
    uint32_t shoresidePassed;
    int32_t bestTurismoTime;
    float distanceWalked;
    float distanceDriven;
    int32_t patriotPlaygroundTime;
    int32_t aRideInTheParkTime;
    int32_t grippedTime;
    int32_t multistoryMayhemTime;
    uint32_t peopleSaved;
    uint32_t criminalsKilled;
    int32_t highestParamedicLevel;
    uint32_t firesExtinguished;
    int32_t longestDodoFlight;
    int32_t bombDefusalTime;
    uint32_t rampagesPassed;
    uint32_t totalRampages;
    uint32_t totalMissions;
    uint32_t fastestTime[16];  // not used
    int32_t highestScore[16];
    uint32_t peopleKilledSinceCheckpoint;  // ?
    uint32_t peopleKilledSinceLastBustedOrWasted;
    char lastMissionGXT[8];

    GameStats();
};

struct TextDisplayData {
    // This is set by the final display text command.
    GameString text;
    glm::vec2 position{};

    glm::vec4 colourFG{};
    glm::vec4 colourBG{};
};

/**
 * Data about a blip
 */
struct BlipData {
    int id;

    enum BlipType {
        None = 0,
        Vehicle = 1,
        Character = 2,
        Instance = 3,
        Coord = 4,
        Contact = 5,
        Pickup = 6
    };
    BlipType type;
    GameObjectID target;
    // If target is null then use coord
    glm::vec3 coord{};

    std::string texture;  // Texture for use in the radar
    uint32_t colour = 0;  // Color value (index or RGBA)
    bool dimmed = false;  // Color dimming if not in RGBA mode

    uint16_t size = 3;  // Only used if texture is empty

    uint8_t brightness = 1;  // Don't really know how it is used

    enum DisplayMode { Hide = 0, MarkerOnly = 1, RadarOnly = 2, ShowBoth = 3 };

    /* Should the blip be displayed? */
    DisplayMode display;

    BlipData() : id(-1), type(None), target(0), display(ShowBoth) {
    }

    int getScriptObjectID() const {
        return id;
    }
};

enum class HudFlash {
    Disabled = -1,
    FlashArmor = 3,
    FlashHealth = 4,
    FlashRadar = 8
};

enum class ScriptContactID {
    Luigi = 0,
    Joey = 1,
    Toni = 2,
    Frankie = 3,
    Asuka = 4,
    Kenji = 5,
    Ray = 6,
    Love = 7,
    Maria = 8,
    RaySuburban = 9,
    AsukaSuburban = 10,
    KenjiSuburban = 11,
    EightBall = 12,
    Hispanic = 13,
    Yardie = 14,
    Hoods = 15,
};

struct ScriptContactData {
    uint32_t onMissionOffset;
    uint32_t baseBrief;
};

/**
 * Game and Runtime state
 *
 * Certain fields are initialized by the SCM file, and persisted in saves
 * such as the Contact data, max wanted level, restart locations etc.
 *
 * Other fields are ephemeral such as controller state.
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
    ScriptInt* scriptOnMissionFlag;

    /** Objects created by the current mission */
    std::vector<GameObject*> missionObjects;

    bool overrideNextRestart;
    glm::vec4 nextRestartLocation;
    std::vector<glm::vec4> hospitalRestarts, policeRestarts;
    int hospitalIslandOverride, policeIslandOverride;

    void addHospitalRestart(const glm::vec4 location);
    void addPoliceRestart(const glm::vec4 location);
    void overrideRestart(const glm::vec4 location);
    void cancelRestartOverride();

    enum RestartType { Hospital, Police };

    const glm::vec4 getClosestRestart(RestartType type,
                                      const glm::vec3 playerPosition) const;

    bool fadeIn;
    float fadeStart;
    float fadeTime;
    bool fadeSound;
    glm::u16vec3 fadeColour;

    // @todo fadeOut should be replaced with enum?
    void fade(float time, bool f);
    bool isFading() const;
    void setFadeColour(glm::i32vec3 colour);

    std::string currentSplash;

    bool skipCutscene;
    bool isIntroPlaying;
    CutsceneData* currentCutscene;
    float cutsceneStartTime;
    /** Flag for rendering cutscene letterbox */
    bool isCinematic;
    HudFlash hudFlash;

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

    /**
     * Script timer
     */
    ScriptInt* scriptTimerVariable = nullptr;
    bool scriptTimerPaused = false;

    /** The camera near value currently set by the script */
    float cameraNear;
    bool cameraFixed;
    glm::vec3 cameraPosition;
    glm::quat cameraRotation;

    GameObjectID cameraTarget;

    std::vector<VehicleGenerator> vehicleGenerators;

    std::map<int, BlipData> radarBlips;

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

    std::array<ScriptContactData, 16> scriptContacts = {};

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

    /**
     * Adds help message
     */
    void showHelpMessage(const GameStringKey& id);
};

#endif
