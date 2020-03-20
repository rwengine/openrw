#ifndef RWGAME_RWIMGUI_HPP
#define RWGAME_RWIMGUI_HPP

#include <SDL.h>

class GameBase;
struct ImGuiContext;
class ViewCamera;

class RWImGui {
    GameBase &_game;
    ImGuiContext *_context = nullptr;
public:
    RWImGui(GameBase &game);
    ~RWImGui();
    void init();
    void destroy();
    bool processEvent(SDL_Event &event);
    void startFrame();
    void endFrame();
};

#endif // RWGAME_RWIMGUI_HPP
