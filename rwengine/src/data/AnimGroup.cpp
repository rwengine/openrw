#include "AnimGroup.hpp"

AnimGroup::AnimGroup(AnimationSet &animations, const std::string &name)
    : idle(find(animations, name, "idle_stance"))
    , walk(find(animations, name, "walk_player"))
    , walk_start(find(animations, name, "walk_start"))
    , run(find(animations, name, "run_player"))
    , sprint(find(animations, name, "sprint_civi"))
    , walk_right(find(animations, name, "walk_player_right"))
    , walk_right_start(find(animations, name, "walk_start_right"))
    , walk_left(find(animations, name, "walk_player_left"))
    , walk_left_start(find(animations, name, "walk_start_left"))
    , walk_back(find(animations, name, "walk_player_back"))
    , walk_back_start(find(animations, name, "walk_start_back"))
    , jump_start(find(animations, name, "jump_launch"))
    , jump_glide(find(animations, name, "jump_glide"))
    , jump_land(find(animations, name, "jump_land"))
    , car_sit(find(animations, name, "car_sit"))
    , car_sit_low(find(animations, name, "car_lsit"))
    , car_open_lhs(find(animations, name, "car_open_lhs"))
    , car_getin_lhs(find(animations, name, "car_getin_lhs"))
    , car_getout_lhs(find(animations, name, "car_getout_lhs"))
    , car_pullout_lhs(find(animations, name, "car_pullout_lhs"))
    , car_jacked_lhs(find(animations, name, "car_jackedlhs"))
    , car_open_rhs(find(animations, name, "car_open_rhs"))
    , car_getin_rhs(find(animations, name, "car_getin_rhs"))
    , car_getout_rhs(find(animations, name, "car_getout_rhs"))
    , car_pullout_rhs(find(animations, name, "car_pullout_rhs"))
    , car_jacked_rhs(find(animations, name, "car_jackedrhs"))
    , kd_front(find(animations, name, "kd_front"))
    , ko_shot_front(find(animations, name, "ko_shot_front")) {
}

Animation *AnimGroup::find(AnimationSet &animations, const std::string &group,
                           const std::string &anim) {
    // @todo actually find the correct animation
    RW_UNUSED(group);
    return animations[anim];
}
