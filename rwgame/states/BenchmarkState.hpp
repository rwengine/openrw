#ifndef _RWGAME_BENCHMARKSTATE_HPP_
#define _RWGAME_BENCHMARKSTATE_HPP_

#include "State.hpp"

class BenchmarkState : public State {
    struct TrackPoint {
        float time;
        glm::vec3 position;
        glm::quat angle;
    };
    std::vector<TrackPoint> track;

    ViewCamera trackCam;

    std::string benchfile;

    float benchmarkTime;
    float duration;
    uint32_t frameCounter;

public:
    BenchmarkState(RWGame* game, const std::string& benchfile);

    virtual void enter() override;
    virtual void exit() override;

    virtual void tick(float dt) override;
    virtual void draw(GameRenderer* r) override;

    virtual void handleEvent(const SDL_Event& event) override;

    const ViewCamera& getCamera();
};

#endif
