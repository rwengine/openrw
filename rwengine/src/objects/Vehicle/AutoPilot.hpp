#ifndef _RWENGINE_AUTOPILOT_HPP_
#define _RWENGINE_AUTOPILOT_HPP_

#include <glm/vec3.hpp>

#include <array>
#include <cstdint>

enum DrivingStyle : int8_t {
    DRIVINGMODE_STOPFORCARS = 0x0,
    DRIVINGMODE_SLOWDOWNFORCARS = 0x1,
    DRIVINGMODE_AVOIDCARS = 0x2,
    DRIVINGMODE_PLOUGHTHROUGH = 0x3,
    DRIVINGMODE_STOPFORCARS_IGNORELIGHTS = 0x4,
};

enum CarMission : int8_t {
    MISSION_NONE = 0x0,
    MISSION_CRUISE = 0x1,
    MISSION_RAMPLAYER_FARAWAY = 0x2,
    MISSION_RAMPLAYER_CLOSE = 0x3,
    MISSION_BLOCKPLAYER_FARAWAY = 0x4,
    MISSION_BLOCKPLAYER_CLOSE = 0x5,
    MISSION_BLOCKPLAYER_HANDBRAKESTOP = 0x6,
    MISSION_WAITFORDELETION = 0x7,
    MISSION_GOTOCOORDS = 0x8,
    MISSION_GOTOCOORDS_STRAIGHT = 0x9,
    MISSION_EMERGENCYVEHICLE_STOP = 0xA,
    MISSION_STOP_FOREVER = 0xB,
    MISSION_GOTOCOORDS_ACCURATE = 0xC,
    MISSION_GOTO_COORDS_STRAIGHT_ACCURATE = 0xD,
    MISSION_GOTOCOORDS_ASTHECROWSWIMS = 0xE,
    MISSION_RAMCAR_FARAWAY = 0xF,
    MISSION_RAMCAR_CLOSE = 0x10,
    MISSION_BLOCKCAR_FARAWAY = 0x11,
    MISSION_BLOCKCAR_CLOSE = 0x12,
    MISSION_BLOCKCAR_HANDBRAKESTOP = 0x13,
    MISSION_HELI_FLYTOCOORS = 0x14,
    MISSION_ATTACKPLAYER = 0x15,
    MISSION_PLANE_FLYTOCOORS = 0x16,
    MISSION_HELI_LAND = 0x17,
    MISSION_SLOWLY_DRIVE_TOWARDS_PLAYER_1 = 0x18,
    MISSION_SLOWLY_DRIVE_TOWARDS_PLAYER_2 = 0x19,
    MISSION_BLOCKPLAYER_FORWARDANDBACK = 0x1A,
};

enum TempAction : int8_t {
    TEMPACT_NONE = 0x0,
    TEMPACT_WAIT = 0x1,
    TEMPACT_REVERSE = 0x2,
    TEMPACT_HANDBRAKETURNLEFT = 0x3,
    TEMPACT_HANDBRAKETURNRIGHT = 0x4,
    TEMPACT_HANDBRAKESTRAIGHT = 0x5,
    TEMPACT_TURNLEFT = 0x6,
    TEMPACT_TURNRIGHT = 0x7,
    TEMPACT_GOFORWARD = 0x8,
    TEMPACT_SWERVELEFT = 0x9,
    TEMPACT_SWERVERIGHT = 0xA,
};

class VehicleObjet;
struct AIGraphNode;

struct AutoPilot {
    // RE stuff
    // Please don't modify order, add or remove objects here!
    // If member is unknown and unused, then it should be also commented
    // If member is replaced by something else, then you should point/link to
    // that "something else" using "see ..."

    int currentNodeId;           // int m_nCurrentNodeId;
    int nextNodeId;              // int m_nNextNodeId;
    int previousNodeId;          // int m_nPreviousNodeId;
    int totalSpeedScaleFactor;   // int m_nTotalSpeedScaleFactor;
    int speedScaleFactor;        // int m_nSpeedScaleFactor;
    int currentCarPathLink;      // int m_nCurrentCarPathLink;
    int nextCarPathLink;         // int m_nNextCarPathLink;
    int previousCarPathLink;     // int m_nPreviousCarPathLink;
    int stopCommandTime;         // int m_nStopCommandTime;
    int lastCommandTime;         // int m_nLastCommandTime;
    char previousDirection;      // char m_bPreviousDirection;
    char currentDirecton;        // char m_bCurrentDirecton;
    char nextDirection;          // char m_bNextDirection;
    char previousPathDirection;  // char m_bPreviousPathDirection;
    char currentPathDirection;   // char m_bCurrentPathDirection;
    DrivingStyle drivingStyle;   // eDrivingStyle m_eDrivingStyle;
    CarMission carMission;       // eCarMission m_eCarMission;
    TempAction tempAction;       // eTempAction m_eTempAction;
    int timeSimpleAction;        // int m_nTimeSimpleAction;
    float speedChange;           // float m_fSpeedChange;
    char maxSpeed;               // char m_bMaxSpeed;
    uint8_t flags;               // uint8_t m_bFlags;
    // char field_358;
    // char field_359;
    glm::vec3 vecGotoCoors;                 // CVec m_vecGotoCoors;
    std::array<AIGraphNode*, 8> pathNodes;  // CPathNode* m_pPathNode[8];
    // int16_t m_nTotalPathNodes;
    // char field_406;
    // char field_407;
    VehicleObjet* target;  // CVehicle *m_pTarget;

    // end of RE stuff
};

#endif
