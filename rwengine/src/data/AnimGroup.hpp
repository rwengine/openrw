#ifndef RWENGINE_DATA_ANIMGROUP_HPP
#define RWENGINE_DATA_ANIMGROUP_HPP
#include <memory>
#include <unordered_map>
#include "rw/types.hpp"

struct Animation;

struct AnimGroup {
    /* Animations */
    Animation* idle;
    Animation* walk;
    Animation* walk_start;
    Animation* run;
    Animation* sprint;

    Animation* walk_right;
    Animation* walk_right_start;
    Animation* walk_left;
    Animation* walk_left_start;

    Animation* walk_back;
    Animation* walk_back_start;

    Animation* jump_start;
    Animation* jump_glide;
    Animation* jump_land;

    Animation* car_sit;
    Animation* car_sit_low;

    Animation* car_open_lhs;
    Animation* car_getin_lhs;
    Animation* car_getout_lhs;
    Animation* car_pullout_lhs;
    Animation* car_jacked_lhs;

    Animation* car_open_rhs;
    Animation* car_getin_rhs;
    Animation* car_getout_rhs;
    Animation* car_pullout_rhs;
    Animation* car_jacked_rhs;

    Animation* kd_front;
    Animation* ko_shot_front;

    AnimGroup(AnimationSet& animations, const std::string&);

    static Animation* find(AnimationSet&, const std::string& group,
                           const std::string& anim);
};

using AnimGroups = std::unordered_map<std::string, std::unique_ptr<AnimGroup>>;

#endif
