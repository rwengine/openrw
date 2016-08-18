#ifndef _RWGAME_BENCHMARKSTATE_HPP_
#define _RWGAME_BENCHMARKSTATE_HPP_

#include "State.hpp"

class BenchmarkState : public State
{
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

  virtual void enter();
  virtual void exit();

  virtual void tick(float dt);
  virtual void draw(GameRenderer* r);

  virtual void handleEvent(const SDL_Event& event);

  const ViewCamera& getCamera();
};

#endif
