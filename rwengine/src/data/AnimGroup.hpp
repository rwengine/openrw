#ifndef _RWENGINE_DATA_ANIMGROUP_HPP_
#define _RWENGINE_DATA_ANIMGROUP_HPP_

#include <algorithm>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <string>
#include <unordered_map>

#include <rw/forward.hpp>

/**
 * The logical animations
 */
enum class AnimCycle {
    Walk = 0,
    Run,
    Sprint,
    Idle,
    WalkStart,
    RunStop,
    RunStopR,
    IdleCam,
    IdleHbhb,
    IdleTired,
    IdleArmed,
    IdleChat,
    IdleTaxi,
    KnockOutShotFront0,
    KnockOutShotFront1,
    KnockOutShotFront2,
    KnockOutShotFront3,
    KnockOutShotFace,
    KnockOutShotStomach,
    KnockOutShotArmL,
    KnockOutShotArmR,
    KnockOutShotLegL,
    KnockOutShotLegR,
    KnockDownLeft,
    KnockDownRight,
    KnockOutSkidFront,
    KnockOutSpinR,
    KnockOutSkidBack,
    KnkockOutSpinL,
    ShotPartial0,
    ShotLeftP,
    ShotPartial1,
    ShotRightP,
    HitFront,
    HitLeft,
    HitBack,
    HitRight,
    FloorHit,
    HitBodyBlow,
    HitChest,
    HitHead,
    HitWalk,
    HitWall,
    FloorHitF,
    HitBehind,
    PunchR,
    KickFloor,
    WeaponBatH,
    WeaponBatV,
    WeaponHgunBody,
    WeaponAKBody,
    WeaponPump,
    WeaponSniper,
    WeaponThrow,
    WeaponThrowu,
    WeaponStartThrow,
    WeaponBomber,
    WeaponHgunReload,
    WeaponAKReload,
    FPSPunch,
    FPSBat,
    FPSUzi,
    FPSPump,
    FPSAK,
    FPSM16,
    FPSRocket,
    FightIdle0,
    FightIdle1,
    FightSh_F,
    FightBodyBlow,
    FightHead,
    FightKick,
    FightKnee,
    FightLHook,
    FightPunch,
    FightRoundhouse,
    FightLongKick,
    FightPPunch,
    CarJackedRHS,
    CarLowJackedRHS,
    CarJackedLHS,
    CarLowJackedLHS,
    CarQuickJack,
    CarQuickJacked,
    CarAlignLHS,
    CarAlignHighLHS,
    CarOpenLHS,
    CarDoorLockedLHS,
    CarPullOutLHS,
    CarLowPullOutLHS,
    CarGetInLHS,
    CarLowGetInLHS,
    CarCloseDoorLHS,
    CarLowCloseDoorLHS,
    CarRollDoor,
    CarLowRollDoor,
    CarGetOutLHS,
    CarLowGetOutLHS,
    CarCloseLHS,
    CarAlignRHS,
    CarAlignHighRHS,
    CarOpenRHS,
    CarDoorLockedRHS,
    CarPullOutRHS,
    CarLowPullOutRHS,
    CarGetInRHS,
    CarLowGetInRHS,
    CarCloseDoorRHS,
    CarLowCloseDoorRHS,
    CarShuffleRHS,
    CarLowShuffleRHS,
    CarSit,
    CarLowSit,
    CarSitPassenger,
    CarLowSitPassenger,
    DriveLeft,
    DriveRight,
    LowDriveLeft,
    LowDriveRight,
    DriveByLeft,
    DriveByRight,
    CarLB,  // ?
    DriveBoat,
    CarGetOutRHS,
    CarLowGetOutRHS,
    CarCloseRHS,
    CarHookerTalk,
    CoachOpenLHS,
    CoachOpenRHS,
    CoachGetInLHS,
    CoachGetInRHS,
    CoachGetOutLHS,
    TrainGetIn,
    TrainGetOut,
    CarCrawlOutLHS,
    CarCrawlOutRHS,
    VanOpenLHS,
    VanGetInLHS,
    VanCloseLHS,
    VanGetOutLHS,
    VanOpenRHS,
    VanGetInRHS,
    VanCloseRHS,
    VanGetOutRHS,
    GetUp0,
    GetUp1,
    GetUp2,
    GetUpFront,
    JumpLaunch,
    JumpGlide,
    JumpLand,
    FallFall,
    FallGlide,
    FallLand,
    FallCollapse,
    EVStep,
    EVDive,
    XpressScratch,
    RoadCross,
    Turn180,
    ArrestGun,
    Drown,
    CPR,
    DuckDown,
    DuckLow,
    RBlockCShoot,
    WeaponThrowu1,
    HandsUp,
    HandsCower,
    FuckYou,
    PhoneIn,
    PhoneOut,
    PhoneTalk,
    _CycleCount
};

struct AnimCycleInfo {
    enum Flags { Repeat = 0x02, Movement = 0x40, Movement_X = 0x1000 };

    AnimCycle id = AnimCycle::Idle;
    /// The name from animgrp.dat or hard-coded
    std::string name;
    /// Flags
    uint32_t flags;
    /// The actual animation
    AnimationPtr anim = nullptr;

    AnimCycleInfo(const std::string& name = "", uint32_t flags = 0)
        : name(name), flags(flags) {
    }
};

struct AnimGroup {
    std::string name_;

    /* Animations */
    AnimCycleInfo animations_[static_cast<uint32_t>(AnimCycle::_CycleCount)];

    AnimationPtr animation(AnimCycle cycle) const {
        return animations_[static_cast<uint32_t>(cycle)].anim;
    }

    uint32_t flags(AnimCycle cycle) const {
        return animations_[static_cast<uint32_t>(cycle)].flags;
    }

    static AnimGroup getBuiltInAnimGroup(AnimationSet&,
                                         const std::string& name);

    static uint32_t getAnimationFlags(const std::string& animation);

    AnimGroup(const std::string& name,
              const std::initializer_list<AnimCycleInfo>& cycles = {})
        : name_(name) {
        std::copy(std::begin(cycles), std::end(cycles),
                  std::begin(animations_));
    }
};

using AnimGroups = std::unordered_map<std::string, std::unique_ptr<AnimGroup>>;

#endif
