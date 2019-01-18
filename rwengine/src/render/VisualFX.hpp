#ifndef _RWENGINE_VISUALFX_HPP_
#define _RWENGINE_VISUALFX_HPP_

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <gl/TextureData.hpp>

enum EffectType { Light, Particle, Trail };

/**
 * Represents a scene effect: lighting, particles etc.
 */
struct VisualFX {
    VisualFX() = default;
    virtual ~VisualFX() = default;

    virtual EffectType getType() const = 0;

    /** Initial world position */
    glm::vec3 position{};
};

struct LightFX final : public VisualFX {
    LightFX() = default;
    ~LightFX() override = default;

    EffectType getType() const override {
        return Light;
    }
};

struct ParticleFX final : public VisualFX {
    /** Direction of particle */
    glm::vec3 direction{};

    /** Particle orientation modes */
    enum Orientation {
        Free,    /** faces direction using up */
        Camera,  /** Faces towards the camera @todo implement */
        UpCamera /** Face closes point in camera's look direction */
    };
    Orientation orientation{Free};

    /** Game time at particle instantiation */
    float starttime{0.f};
    /** Number of seconds particle should exist for, negative values =
     * forever */
    float lifetime{-1.f};

    /** Texture name */
    TextureData* texture = nullptr;

    /** Size of particle */
    glm::vec2 size{1.f, 1.f};

    /** Up direction (only used in Free mode) */
    glm::vec3 up{0.f, 0.f, 1.f};

    /** Render tint colour */
    glm::vec4 colour{1.f, 1.f, 1.f, 1.f};

    /** Constructs a particle */
    ParticleFX() = default;
    ~ParticleFX() override = default;

    EffectType getType() const override {
        return Particle;
    }
};

struct TrailFX final : public VisualFX {
    TrailFX() = default;
    ~TrailFX() = default;

    EffectType getType() const override {
        return Trail;
    }
};

#endif
