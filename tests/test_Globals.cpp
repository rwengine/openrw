#include "test_Globals.hpp"

#include <GameConfig.hpp>

std::ostream& operator<<(std::ostream& stream, const glm::vec3& v) {
    stream << v.x << " " << v.y << " " << v.z;
    return stream;
}

Global::Global() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        throw std::runtime_error("Failed to initialize SDL2!");

    window.create("Tests", 800, 600, false);
    window.hideCursor();

#if RW_TEST_WITH_DATA
    d = new GameData(&log, getGamePath());

    d->load();

    e = new GameWorld(&log, d);
    s = new GameState;
    e->state = s;

    e->dynamicsWorld->setGravity(btVector3(0.f, 0.f, 0.f));
#endif
}

Global::~Global() {
    window.close();
#if RW_TEST_WITH_DATA
    delete e;
#endif
}

#if RW_TEST_WITH_DATA
std::string Global::getGamePath() {
    GameConfig config;
    config.loadFile(GameConfig::getDefaultConfigPath() / "openrw.ini");
    return config.getGameDataPath().string(); //FIXME: use path
}
#endif
