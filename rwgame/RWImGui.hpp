#ifndef RWGAME_RWIMGUI_HPP
#define RWGAME_RWIMGUI_HPP

#include <SDL.h>

class RWGame;
struct ImGuiContext;
class ViewCamera;

class RWImGui {
    RWGame &_game;
    ImGuiContext *_context = nullptr;
public:
    RWImGui(RWGame &game);
    ~RWImGui();
    void init();
    void destroy();
    bool processEvent(SDL_Event &event);
    void startFrame();
    void endFrame(const ViewCamera &);
};

#endif // RWGAME_RWIMGUI_HPP
