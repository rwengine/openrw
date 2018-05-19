#ifndef _RWGAME_BENCHMARKSTATE_HPP_
#define _RWGAME_BENCHMARKSTATE_HPP_

#include "State.hpp"

class BenchmarkState : public State {
    struct TrackPoint {
        float time;
        glm::vec3 position{};
        glm::quat angle{1.0f,0.0f,0.0f,0.0f};
    };
    std::vector<TrackPoint> track;

    ViewCamera trackCam;

    std::string benchfile;

    float benchmarkTime{0.f};
    float duration{0.f};
    uint32_t frameCounter{0};

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
