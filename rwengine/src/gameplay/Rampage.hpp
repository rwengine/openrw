#ifndef _RWENGINE_RAMPAGE_HPP_
#define _RWENGINE_RAMPAGE_HPP_

#include <rw/defines.hpp>

class Rampage {
public:
    void start();
    void tick(float dt);

private:
    bool onGoing;
    int registeredKills;

    int model0ToKill;
    int model1ToKill;
    int model2ToKill;
    int model3ToKill;

    int weaponIdRequired;
    int killsRequired;
    int timeLimit;

    bool showMessagesAndPlaySound;

    bool headshotsOnly;
};

#endif