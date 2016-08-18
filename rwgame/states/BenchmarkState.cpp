#include "BenchmarkState.hpp"
#include "RWGame.hpp"
#include <engine/GameState.hpp>
#include <fstream>

BenchmarkState::BenchmarkState(RWGame* game, const std::string& benchfile)
    : State(game), benchfile(benchfile), benchmarkTime(0.f), frameCounter(0)
{
}

void BenchmarkState::enter()
{
  getWindow().hideCursor();

  std::ifstream benchstream(benchfile);

  unsigned int clockHour;
  unsigned int clockMinute;
  benchstream >> clockHour;
  benchstream.seekg(1, std::ios_base::cur);
  benchstream >> clockMinute;

  game->getWorld()->state->basic.gameHour = clockHour;
  game->getWorld()->state->basic.gameMinute = clockMinute;

  float time = 0.f;
  glm::vec3 tmpPos;
  while (benchstream) {
    TrackPoint point;
    benchstream >> point.time;
    if (!benchstream) break;
    benchstream >> point.position.x;
    if (!benchstream) break;
    benchstream >> point.position.y;
    if (!benchstream) break;
    benchstream >> point.position.z;
    if (!benchstream) break;
    benchstream >> point.angle.x;
    if (!benchstream) break;
    benchstream >> point.angle.y;
    if (!benchstream) break;
    benchstream >> point.angle.z;
    if (!benchstream) break;
    benchstream >> point.angle.w;
    if (!benchstream) break;
    if (track.size() == 0) {
      tmpPos = point.position;
    }
    float pointDist = glm::distance(tmpPos, point.position);
    tmpPos = point.position;
    point.time = time + pointDist / 50.f;
    time = point.time;
    duration = std::max(duration, point.time);
    track.push_back(point);
  }

  std::cout << "Loaded " << track.size() << " points" << std::endl;
}

void BenchmarkState::exit()
{
  std::cout << "Results =============\n"
            << "Benchmark: " << benchfile << "\n"
            << "Frames: " << frameCounter << "\n"
            << "Duration: " << duration << " seconds\n"
            << "Avg frametime: " << std::setprecision(3) << (duration / frameCounter) << " ("
            << (frameCounter / duration) << " fps)" << std::endl;
}

void BenchmarkState::tick(float dt)
{
  if (track.size() > 0) {
    TrackPoint& a = track.front();
    TrackPoint& b = track.back();
    for (TrackPoint& p : track) {
      if (benchmarkTime < p.time) {
        b = p;
        break;
      }
      a = p;
    }
    if (benchmarkTime > duration) {
      StateManager::get().exit();
    }
    if (b.time != a.time) {
      float alpha = (benchmarkTime - a.time) / (b.time - a.time);
      trackCam.position = glm::mix(a.position, b.position, alpha);
      trackCam.rotation = glm::slerp(a.angle, b.angle, alpha);
    }
    benchmarkTime += dt;
  }
}

void BenchmarkState::draw(GameRenderer* r)
{
  frameCounter++;
  State::draw(r);
}

void BenchmarkState::handleEvent(const SDL_Event& e) { State::handleEvent(e); }

const ViewCamera& BenchmarkState::getCamera() { return trackCam; }
