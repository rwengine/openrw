#ifndef _RWENGINE_SOUNDMANAGER_HPP_
#define _RWENGINE_SOUNDMANAGER_HPP_

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <glm/glm.hpp>

#include <al.h>
#include <alc.h>

#include "audio/Sound.hpp"

/// Script is using different numeration of sounds
/// than postion index in sfx file.
/// Also it is needed to store range of sound.
/// Struct is used by opcodes 018c, 018d.
struct SoundInstanceData {
    int id;
    int sfx;
    int range;
};
/// Hardcoded array for translation script index into sfx index
/// (In origin it also had been hardcoded)
/// (-1 means unlimited)
/// @todo some of them should return
/// random(?) number in range
/// see comment in second column
constexpr SoundInstanceData sfxData[] = {
    /// for opcodes 018c
    {78, 2857/*-2871*/, -1}, /// male pain soft
    {79, 2857/*-2871*/, -1}, /// male pain loud
    {80, 2290/*-2300*/, -1}, /// female pain soft
    {81, 2290/*-2300*/, -1}, /// female pain loud
    {82, 161/*-162*/, -1}, /// item pickup
    {83, 161/*-162*/, -1}, /// item pickup
    {92, 147, 40}, /// gate start
    {93, 147, 40}, /// gate stop
    {94, 337, -1}, /// checkpoint
    {97, 336, -1}, /// countdown timer 3
    {98, 336, -1}, /// countdown timer 2
    {99, 336, -1}, /// countdown timer 1
    {100, 337, -1}, /// countdown finish
    {106/*-108*/, 176/*-178*/, 50}, /// bullet hit ground
    {110/*-111*/, 389, 80},  /// silent, replace the SFX or edit the memory to a different SFX to make this useful
    {112, 283, 80}, /// payphone
    {114/*-115*/, 151, 60}, /// glass break
    {116, 150, 60}, /// glass damage
    {117, 152/*-155*/, 55}, /// glass shards
    {118, 327, 60}, /// boxes destroyed
    {119, 328, 60}, /// boxes destroyed
    {120, 140/*-144*/, 60}, /// car collision
    {121, 29, 60}, /// tire collision
    {122, 167/*-168*/, 20}, /// gun shell drop, dependent on what collision the player is currently standing on
    {123, 168, 20}, /// gun shell drop soft
    /// for opcodes 018d
    {4, 390, 30},
    {5, 390, 80}, /// 	Hepburn Heights southwest tower
    {6, 391, 30},
    {7, 391, 80}, /// 	Hepburn Heights north tower
    {8, 392, 30},
    {9, 392, 80},
    {10, 393, 30},
    {11, 393, 80},
    {12, 394, 30},
    {13, 394, 80},
    {14, 395, 30},
    {15, 395, 80},
    {16, 396, 30},
    {17, 396, 80},
    {18, 397, 30},
    {19, 397, 80},
    {20, 398, 30},
    {21, 398, 80},
    {22, 399, 30},
    {23, 399, 80},
    {24, 390, 30},
    {25, 390, 80},
    {26, 391, 30},
    {27, 391, 80},
    {28, 392, 30},
    {29, 392, 80},
    {30, 403, 30}, /// Meeouch Sex Kitten Club
    {31, 403, 80},
    {32, 404, 30}, /// 	Sex Club Seven
    {33, 404, 80},
    {34, 405, 30},
    {35, 405, 30},
    {36, 407/*-408*/, 30}, /// 		407 plays continuously while 408 plays intermittently
    {37, 407/*-408*/, 30}, /// 	Liberty City Sawmills/Bitch'N' Dog Food
    {38, 409, 30},
    {39, 409, 80},
    {40, 410/*-411*/, 30}, /// 		Both plays continuously
    {41, 410/*-411*/, 30}, /// 	Mr. Wong's Laundrette
    {42, 412, 30}, /// 	Roast Peking Duck
    {43, 412, 80},
    {44, 413, 30}, /// 	Cipriani's Ristorante
    {45, 413, 80},
    {46, 414, 30},
    {47, 414, 30},
    {48, 415, 30}, /// 	Marco's Bistro (no sound)
    {49, 415, 80},
    {50, 416/*-419*/, 30}, /// 		Plays separately and intermittently
    {51, 416/*-419*/, 80}, /// 	Francis International Terminal B
    {52, 420/*-422*/, 30},
    {53, 420/*-422*/, 30}, /// Chinatown (no sound)
    {54, 423/*-425*/, 30}, /// 		Plays separately and intermittently
    {55, 423/*-425*/, 80},
    {56, 426, 30},
    {57, 426, 80}, /// 	Portland Docks (no sound)
    {58, 427/*-431*/, 30}, /// 		Plays separately and intermittently
    {59, 427/*-431*/, 80}, /// Left side of Misty's apartment
    {60, 406, 30}, /// 	Salvatore's place
    {61, 406, 80},
    {62, 432/*-434*/, 20},  /// South of Sex Club Seven 	432 plays continuously while 433-434 plays separately and intermittently
    {63, 432/*-434*/, 80},
    {64, 435/*-437*/, 20}, /// 	East of Portland Pay 'N' Spray 	435 plays continuously while 436-437 plays separately and intermittently
    {65, 435/*-437*/, 80},
    {66, 438/*-440*/, 20}, /// 	Executive Relief 	438 plays continuously while 439-440 plays separately and intermittently
    {67, 438/*-440*/, 80},
    {68, 442, 30},
    {69, 442, 80}, /// 	Bank of Liberty/Staunton police HQ alarm
    {70, 441, 30},
    {71, 441, 80}, /// 	Old school hall
    {72, 443, 30},
    {73, 443, 80}, /// 	Warehouse rave
    {76, 179/*-184*/, 30}, /// 		Plays separately and intermittently
    {77, 179/*-184*/, 80}, /// 	Staunton police HQ
    {84, 444, 30},
    {85, 444, 80},
    {86, 444, 30},
    {87, 444, 80},
    {88, 445, 30},
    {89, 445, 80},
    {90, 433/*-434*/, 20}, /// 		Plays separately and intermittently
    {91, 433/*-434*/, 80}, /// 	Right side of Misty's apartment
    {102, 157, 50} /// 	Callahan Bridge fire
};
/// Nr of existing sfx sounds
constexpr size_t kNrOfAllSfx = 3032;

/// Game's sound manager.
/// It handles all staff conntected with sounds.
/// Worth noted: there are three types of sounds,
/// these containg raw source and openAL buffer for playing (only one instance simultaneously),
/// these containg only source or buffer.
/// (It allows multiple instances simultaneously without duplicating raw source).
class SoundManager {
public:
    SoundManager();
    ~SoundManager();

    /// Load sound from file and store it with selected name
    bool loadSound(const std::string& name, const std::string& fileName);

    /// Load all sfx sounds
    void loadSfxSounds(const std::string& path);

    /// Get position index in sdt file
    int convertScriptIndexIntoSfx(const int scriptId);
    Sound& getSoundRef(const size_t& name);
    Sound& getSoundRef(const std::string& name);

    /// Get range of sound.
    /// (arg must be script id)
    int getScriptRange(const int scriptId);
    size_t  createSfxInstance(const size_t& index);

    /// Checking is selected sound loaded.
    bool isLoaded(const std::string& name);

    /// Checking is selected sound playing.
    bool isPlaying(const std::string& name);

    /// Checking is selected sound playing.
    bool isStoped(const std::string& name);

    /// Checking is selected sound playing.
    bool isPaused(const std::string& name);

    /// Play sound with selected name
    void playSound(const std::string& name);

    /// Effect same as playSound with one parametr,
    /// but this function works for sfx and
    /// allows also for setting position,
    /// looping and max Distance.
    /// -1 means no limit of max distance.
    void playSfx(const size_t& name, const glm::vec3 position, const bool looping = false, const int maxDist = -1);

    void pauseAllSounds();
    void resumeAllSounds();

    /// Play background from selected file.
    bool playBackground(const std::string& fileName);

    bool loadMusic(const std::string& name, const std::string& fileName);
    void playMusic(const std::string& name);
    void stopMusic(const std::string& name);

    /// Setting position of listener for openAL.
    void setListenerPosition(const glm::vec3 position);

    /// Setting velocity of velocity for openAL.
    void setListenerVelocity(const glm::vec3 vel);

    /// Setting orientation of listener for openAL.
    /// Worth noted v = { at.x, at.y, at.z, up.x, up.y, up.z}
    void setListenerOrientation(const glm::vec3 at);

    /// Setting position of sound source in buffer.
    void setSoundPosition(const std::string name,const glm::vec3 position);

    void pause(bool p);

private:
    bool initializeOpenAL();
    bool initializeAVCodec();

    ALCcontext* alContext = nullptr;
    ALCdevice* alDevice = nullptr;

    /// Containers for sounds
    std::unordered_map<std::string, Sound> sounds;
    std::unordered_map<size_t, Sound> sfx;
    std::unordered_map<size_t, Sound> buffers;

    std::string backgroundNoise;

    /// Nr of already created buffers
    size_t bufforNr = 0;
};

#endif
