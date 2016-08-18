#pragma once

#include <glm/glm.hpp>
#include <gl/TextureData.hpp>

/**
 * Represents a scene effect: lighting, particles etc.
 */
class VisualFX
{
public:
  enum EffectType { Light, Particle, Trail };

  struct LightData {
    ~LightData();
  };
  struct ParticleData {
    /** Initial world position */
    glm::vec3 position;

    /** Direction of particle */
    glm::vec3 direction;

    /** Particle orientation modes */
    enum Orientation {
      Free,    /** faces direction using up */
      Camera,  /** Faces towards the camera @todo implement */
      UpCamera /** Face closes point in camera's look direction */
    };
    Orientation orientation;

    /** Game time at particle instantiation */
    float starttime;
    /** Number of seconds particle should exist for, negative values = forever */
    float lifetime;

    /** Texture name */
    TextureData::Handle texture;

    /** Size of particle */
    glm::vec2 size;

    /** Up direction (only used in Free mode) */
    glm::vec3 up;

    /** Render tint colour */
    glm::vec4 colour;

    /** Constructs a particle */
    ParticleData()
        : orientation(Free),
          starttime(0.f),
          lifetime(-1.f),
          size(1.f, 1.f),
          up(0.f, 0.f, 1.f),
          colour(1.f, 1.f, 1.f, 1.f)
    {
    }
    ~ParticleData();
  };
  struct TrailData {
    ~TrailData();
  };

  /// @todo stop abusing unions
  union {
    LightData light;
    ParticleData particle;
    TrailData trail;
  };

  VisualFX(EffectType type);
  ~VisualFX();

  EffectType getType() const { return type; }

  const glm::vec3& getPosition() const;

private:
  EffectType type;
};
