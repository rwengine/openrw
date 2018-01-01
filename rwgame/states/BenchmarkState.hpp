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

    void enter() override;

    void exit() override;

    void tick(float dt) override;

    void draw(GameRenderer* r) override;

    void handleEvent(const SDL_Event& event) override;

    const ViewCamera& getCamera(float) override;
};

#endif
