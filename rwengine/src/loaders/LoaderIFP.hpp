#ifndef _RWENGINE_LOADERIFP_HPP_
#define _RWENGINE_LOADERIFP_HPP_

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <rw/forward.hpp>

struct AnimationKeyframe {
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 position{};
    glm::vec3 scale{1.0f};
    float starttime = 0.f;
    int id = 0;

    AnimationKeyframe(glm::quat _rotation, glm::vec3 _position,
                      glm::vec3 _scale, float _starttime, int _id)
        : rotation(_rotation)
        , position(_position)
        , scale(_scale)
        , starttime(_starttime)
        , id(_id) {
    }

    AnimationKeyframe() = default;
};

struct AnimationBone {
    std::string name;
    int32_t previous;
    int32_t next;
    float duration;

    enum Data { R00, RT0, RTS };

    Data type;
    std::vector<AnimationKeyframe> frames;

    AnimationBone() = default;

    AnimationBone(const std::string& p_name, int32_t p_previous, int32_t p_next,
                  float p_duration, Data p_type,
                  const std::vector<AnimationKeyframe>& p_frames)
        : name(p_name)
        , previous(p_previous)
        , next(p_next)
        , duration(p_duration)
        , type(p_type)
        , frames(p_frames) {
    }

    ~AnimationBone() = default;

    AnimationKeyframe getInterpolatedKeyframe(float time);
    AnimationKeyframe getKeyframe(float time);
};

/**
 * @brief Animation data object, stores bones.
 *
 * @todo break out into Animation.hpp
 */
struct Animation {
    std::string name;
    std::map<std::string, std::unique_ptr<AnimationBone>> bones;

    ~Animation() = default;

    float duration;
};

class LoaderIFP {
    template<class T>
    T* read(char* data, size_t* ofs) {
        size_t b = *ofs;
        *ofs += sizeof(T);
        return reinterpret_cast<T*>(data + b);
    }
    template<class T>
    T* peek(char* data, const size_t* ofs) {
        return reinterpret_cast<T*>(data + *ofs);
    }

    std::string readString(char* data, size_t* ofs);

public:
    struct BASE {
        char magic[4];
        uint32_t size;
    };

    struct INFO {
        BASE base;
        int32_t entries;
        // null terminated string
        // entry data
    };

    struct ANPK {
        BASE base;
        INFO info;
    };

    struct NAME {
        BASE base;
    };

    struct DGAN {
        BASE base;
        INFO info;
    };

    struct CPAN {
        BASE base;
    };

    struct ANIM {
        BASE base;
        char name[28];
        int32_t frames;
        int32_t unk;
        int32_t next;
        int32_t prev;
    };

    struct KFRM {
        BASE base;
    };

    struct Anim {
        std::string name;
    };

    AnimationSet animations;

    bool loadFromMemory(char* data);
};

#endif
