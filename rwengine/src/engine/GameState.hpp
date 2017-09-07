#ifndef _RWENGINE_GAMESTATE_HPP_
#define _RWENGINE_GAMESTATE_HPP_
#include <bitset>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <data/VehicleGenerator.hpp>
#include <engine/GameData.hpp>
#include <engine/GameInputState.hpp>
#include <engine/GameWorld.hpp>
#include <engine/ScreenText.hpp>
#include <fonts/GameTexts.hpp>
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
    GameStringChar saveName[24]{0};
    SystemTime saveTime{0, 0, 0, 0, 0, 0, 0, 0};
    uint32_t unknown{0};
    uint16_t islandNumber{0};
    glm::vec3 cameraPosition{};
    uint32_t gameMinuteMS{0};
    uint32_t lastTick{0};
    uint8_t gameHour{0};
    uint8_t _align0[3]{0};
    uint8_t gameMinute{0};
    uint8_t _align1[3]{0};
    uint16_t padMode{0};
    uint8_t _align2[2]{0};
    uint32_t timeMS{0};
    float timeScale{1.f};
    float timeStep{0.f};
    float timeStep_unclipped{0};  // Unknown purpose
    uint32_t frameCounter{0};
    float timeStep2{0.f};
    float framesPerUpdate{0.f};
    float timeScale2{0.f};
    uint16_t lastWeather{0};
    uint8_t _align3[2]{0};
    uint16_t nextWeather{0};
    uint8_t _align4[2]{0};
    uint16_t forcedWeather{0};
    uint8_t _align5[2]{0};
    float weatherInterpolation{1.f};
    uint8_t dateTime[24]{0};  // Unused
    uint32_t weatherType{0};
    float cameraData{0};
    float cameraData2{0};

    BasicState() = default;
};

/** Block 16 player info */
struct PlayerInfo {
    int32_t money{0};
    uint8_t unknown1{0};
    uint32_t unknown2{0};
    uint16_t unknown3{0};
    float unknown4{0.f};
    int32_t displayedMoney{0};
    uint32_t hiddenPackagesCollected{0};
    uint32_t hiddenPackageCount{0};
    uint8_t neverTired{0};
    uint8_t fastReload{0};
    uint8_t thaneOfLibertyCity{0};
    uint8_t singlePayerHealthcare{0};
    uint8_t unknown5[70]{0};

    PlayerInfo() = default;
};

/** Block 17 */
struct GameStats {
    uint32_t playerKills{0};
    uint32_t otherKills{0};
    uint32_t carsExploded{0};
    uint32_t shotsHit{0};
    uint32_t pedTypesKilled[23]{};
    uint32_t helicoptersDestroyed{0};
    uint32_t playerProgress{0};
    uint32_t explosiveKgsUsed{0};
    uint32_t bulletsFired{0};
    uint32_t bulletsHit{0};
    uint32_t carsCrushed{0};
    uint32_t headshots{0};
    uint32_t timesBusted{0};
    uint32_t timesHospital{0};
    uint32_t daysPassed{0};
    uint32_t mmRainfall{0};
    uint32_t insaneJumpMaxDistance{0};
    uint32_t insaneJumpMaxHeight{0};
    int32_t insaneJumpMaxFlips{0};
    int32_t insaneJumpMaxRotation{0};
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
    int32_t bestStunt{0};
    uint32_t uniqueStuntsFound{0};
    uint32_t uniqueStuntsTotal{0};
    uint32_t missionAttempts{0};
    uint32_t missionsPassed{0};
    uint32_t passengersDroppedOff{0};
    uint32_t taxiRevenue{0};
    uint32_t portlandPassed{0};
    uint32_t stauntonPassed{0};
    uint32_t shoresidePassed{0};
    int32_t bestTurismoTime{0};
    float distanceWalked{0};
    float distanceDriven{0};
    int32_t patriotPlaygroundTime{0};
    int32_t aRideInTheParkTime{0};
    int32_t grippedTime{0};
    int32_t multistoryMayhemTime{0};
    uint32_t peopleSaved{0};
    uint32_t criminalsKilled{0};
    int32_t highestParamedicLevel{0};
    uint32_t firesExtinguished{0};
    int32_t longestDodoFlight{0};
    int32_t bombDefusalTime{0};
    uint32_t rampagesPassed{0};
    uint32_t totalRampages{0};
    uint32_t totalMissions{0};
    uint32_t fastestTime[16]{0};  // not used
    int32_t highestScore[16]{};
    uint32_t peopleKilledSinceCheckpoint{0};  // ?
    uint32_t peopleKilledSinceLastBustedOrWasted{0};
    char lastMissionGXT[8]{""};

    GameStats() = default;
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
    BasicState basic{};

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
    float gameTime = 0.f;
    unsigned int currentProgress = 0;
    unsigned int maxProgress = 1;

    unsigned int maxWantedLevel = 0;

    GameObjectID playerObject = 0;

    /**
     * @brief Stores a pointer to script global that stores the on-mission
     * state.
     */
    ScriptInt* scriptOnMissionFlag = nullptr;

    /** Objects created by the current mission */
    std::vector<GameObject*> missionObjects;

    bool overrideNextRestart = false;
    glm::vec4 nextRestartLocation{};
    std::vector<glm::vec4> hospitalRestarts, policeRestarts;
    int hospitalIslandOverride = 0;
    int policeIslandOverride = 0;

    void addHospitalRestart(const glm::vec4 location);
    void addPoliceRestart(const glm::vec4 location);
    void overrideRestart(const glm::vec4 location);
    void cancelRestartOverride();

    enum RestartType { Hospital, Police };

    const glm::vec4 getClosestRestart(RestartType type,
                                      const glm::vec3 playerPosition) const;

    bool fadeIn = true;
    float fadeStart = 0.f;
    float fadeTime = 0.f;
    bool fadeSound = false;
    glm::u16vec3 fadeColour{0.f, 0.f, 0.f};

    // @todo fadeOut should be replaced with enum?
    void fade(float time, bool f);
    bool isFading() const;
    void setFadeColour(glm::i32vec3 colour);

    std::string currentSplash;

    bool skipCutscene = false;
    bool isIntroPlaying = false;
    CutsceneData* currentCutscene = nullptr;
    float cutsceneStartTime{-1.f};
    /** Flag for rendering cutscene letterbox */
    bool isCinematic = false;
    HudFlash hudFlash{HudFlash::Disabled};

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

    int bigNVeinyPickupsCollected = 0;

    /** The camera near value currently set by the script */
    float cameraNear{0.1f};
    bool cameraFixed = false;
    glm::vec3 cameraPosition{};
    glm::quat cameraRotation{1.0f, 0.0f, 0.0f, 0.0f};

    GameObjectID cameraTarget = 0;

    std::vector<VehicleGenerator> vehicleGenerators;

    std::map<int, BlipData> radarBlips;

    /**
     * Bitsets for the car import / export list mission
     */
    std::bitset<32> importExportPortland = 0;
    std::bitset<32> importExportShoreside = 0;
    std::bitset<32> importExportUnused = 0;

    /**
     * State of the game input for the last 2 frames
     */
    GameInputState input[2]{};

    /**
     * World to use for this state, this isn't saved, just used at runtime
     */
    GameWorld* world = nullptr;

    /**
     * Script Machine associated with this state if it exists.
     */
    ScriptMachine* script = nullptr;

    std::array<ScriptContactData, 16> scriptContacts = {};

    GameState() = default;

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
