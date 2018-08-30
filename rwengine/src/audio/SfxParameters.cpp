#include "audio/SfxParameters.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>

namespace {
/// Hardcoded array for translating script index into sfx index
/// Valid for gta3.
/// (In origin it also had been hardcoded)
/// (-1 means unlimited)
/// @todo some of them should return
/// random(?) number in range
/// see comment in second column
/// @todo: preparing data for VC and SA
constexpr SoundInstanceData sfxData[] = {
    /// for opcode 018c
    {78, 2857 /*-2871*/, -1},  /// male pain soft
    {79, 2857 /*-2871*/, -1},  /// male pain loud
    {80, 2290 /*-2300*/, -1},  /// female pain soft
    {81, 2290 /*-2300*/, -1},  /// female pain loud
    {82, 161 /*-162*/, -1},    /// item pickup
    {83, 161 /*-162*/, -1},    /// item pickup
    {92, 147, 40},             /// gate start
    {93, 147, 40},             /// gate stop
    {94, 337, -1},             /// checkpoint
    {97, 336, -1},             /// countdown timer 3
    {98, 336, -1},             /// countdown timer 2
    {99, 336, -1},             /// countdown timer 1
    {100, 337, -1},            /// countdown finish
    {106, 176, 50},            /// bullet hit ground
    {107, 177, 50},
    {108, 178, 50},
    {110, 389, 80},  /// silent
    {111, 389, 80},
    {112, 283, 80},           /// payphone
    {114, 151, 60},           /// glass break
    {115, 151, 60},           /// glass break
    {116, 150, 60},           /// glass damage
    {117, 152 /*-155*/, 55},  /// glass shards
    {118, 327, 60},           /// boxes destroyed
    {119, 328, 60},           /// boxes destroyed
    {120, 140 /*-144*/, 60},  /// car collision
    {121, 29, 60},            /// tire collision
    {122, 167 /*-168*/, 20},  /// gun shell drop,
                              /// dependent on collision
                              /// the player is currently
                              /// standing on
    {123, 168, 20},           /// gun shell drop soft
    /// for opcode 018d
    {4, 390, 30},
    {5, 390, 80},  /// Hepburn Heights
                   /// southwest tower
    {6, 391, 30},
    {7, 391, 80},  /// Hepburn Heights
                   /// north tower
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
    {30, 403, 30},  /// Meeouch Sex Kitten Club
    {31, 403, 80},
    {32, 404, 30},  /// Sex Club Seven
    {33, 404, 80},
    {34, 405, 30},
    {35, 405, 30},
    {36, 407 /*-408*/, 30},  /// 407 plays continuously
                             /// while 408 plays intermittently
    {37, 407 /*-408*/, 30},  /// Liberty City
                             /// Sawmills/Bitch'N' Dog Food
    {38, 409, 30},
    {39, 409, 80},
    {40, 410 /*-411*/, 30},  /// Both plays continuously
    {41, 410 /*-411*/, 30},  /// Mr. Wong's Laundrette
    {42, 412, 30},           /// Roast Peking Duck
    {43, 412, 80},
    {44, 413, 30},  /// Cipriani's Ristorante
    {45, 413, 80},
    {46, 414, 30},
    {47, 414, 30},
    {48, 415, 30},  /// Marco's Bistro (no sound)
    {49, 415, 80},
    {50, 416 /*-419*/, 30},  /// Plays separately and intermittently
    {51, 416 /*-419*/, 80},  /// Francis International Terminal B
    {52, 420 /*-422*/, 30},
    {53, 420 /*-422*/, 30},  /// Chinatown (no sound)
    {54, 423 /*-425*/, 30},  /// Plays separately and intermittently
    {55, 423 /*-425*/, 80},
    {56, 426, 30},
    {57, 426, 80},           /// Portland Docks (no sound)
    {58, 427 /*-431*/, 30},  /// Plays separately and intermittently
    {59, 427 /*-431*/, 80},  /// Left side of Misty's apartment
    {60, 406, 30},           /// Salvatore's place
    {61, 406, 80},
    {62, 432 /*-434*/, 20},  /// South of Sex Club Seven
                             /// 432 plays continuously
                             /// while 433-434
                             /// plays separately
                             /// and intermittently
    {63, 432 /*-434*/, 80},
    {64, 435 /*-437*/, 20},  /// East of Portland
                             /// Pay 'N' Spray 435 plays
                             /// continuously while
                             /// 436-437 plays separately
                             /// and intermittently
    {65, 435 /*-437*/, 80},
    {66, 438 /*-440*/, 20},  /// Executive Relief 438
                             /// plays continuously while
                             /// 439-440 plays separately
                             /// and intermittently
    {67, 438 /*-440*/, 80},
    {68, 442, 30},
    {69, 442, 80},  /// Bank of Liberty/Staunton
                    /// police HQ alarm
    {70, 441, 30},
    {71, 441, 80},  /// Old school hall
    {72, 443, 30},
    {73, 443, 80},           /// Warehouse rave
    {76, 179 /*-184*/, 30},  /// Plays separately and intermittently
    {77, 179 /*-184*/, 80},  /// Staunton police HQ
    {84, 444, 30},
    {85, 444, 80},
    {86, 444, 30},
    {87, 444, 80},
    {88, 445, 30},
    {89, 445, 80},
    {90, 433 /*-434*/, 20},  /// Plays separately and intermittently
    {91, 433 /*-434*/, 80},  /// Right side of Misty's apartment
    {102, 157, 50}           /// Callahan Bridge fire
};
}  // namespace

const SoundInstanceData* getSoundInstanceData(int scriptId) {
    const auto found = std::find_if(std::begin(sfxData), std::end(sfxData),
                                    [&scriptId](const SoundInstanceData data) {
                                        return data.id == scriptId;
                                    });
    if (found != std::end(sfxData)) {
        return found;
    }
    return std::begin(sfxData);
}
