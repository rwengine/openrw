#include "render/VisualFX.hpp"

#include <new>

VisualFX::LightData::~LightData() {
}

VisualFX::ParticleData::~ParticleData() {
}

VisualFX::TrailData::~TrailData() {
}

VisualFX::VisualFX(VisualFX::EffectType type) : type(type) {
    switch (type) {
        case VisualFX::Light:
            new (&light) LightData;
            break;
        case VisualFX::Particle:
            new (&particle) ParticleData;
            break;
        case VisualFX::Trail:
            new (&trail) TrailData;
            break;
    }
}

VisualFX::~VisualFX() {
    switch (type) {
        case VisualFX::Light:
            light.~LightData();
            break;
        case VisualFX::Particle:
            particle.~ParticleData();
            break;
        case VisualFX::Trail:
            trail.~TrailData();
            break;
    }
}

const glm::vec3& VisualFX::getPosition() const {
    static glm::vec3 errorRef{};
    switch (type) {
        case VisualFX::Particle:
            return particle.position;
        default:
            return errorRef;
    }
}
