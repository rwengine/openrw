#ifndef RWGAME_RWIMGUI_HPP
#define RWGAME_RWIMGUI_HPP

#include <SDL.h>

class RWGame;
struct ImGuiContext;

class RWImGui {
    RWGame &_game;
    ImGuiContext *_context = nullptr;
public:
    RWImGui(RWGame &game);
    ~RWImGui();
    void init();
    void destroy();
    bool process_event(SDL_Event &event);
    void tick();
};

#endif // RWGAME_RWIMGUI_HPP
