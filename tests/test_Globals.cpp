#include "test_Globals.hpp"

#include <GameConfig.hpp>

#include <btBulletDynamicsCommon.h>
#include <SDL.h>

static std::unique_ptr<Global> global_singleton;
GlobalArgs global_args;

std::ostream& operator<<(std::ostream& stream, const glm::vec3& v) {
    stream << v.x << " " << v.y << " " << v.z;
    return stream;
}

Global::Global(const GlobalArgs& args) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        throw std::runtime_error("Failed to initialize SDL2!");

    window.create("Tests", 800, 600, false);
    window.hideCursor();

    if (args.with_data) {
        d = new GameData(&log, getGamePath());

        d->load();

        e = new GameWorld(&log, d);
        s = new GameState;
        e->state = s;

        e->dynamicsWorld->setGravity(btVector3(0.f, 0.f, 0.f));
    }
}

Global::~Global() {
    window.close();
    delete e;
}

Global& Global::get() {
   if (!global_singleton) {
       global_singleton = std::make_unique<Global>(global_args);
   }
   return *global_singleton;
}

std::string Global::getGamePath() {
    GameConfig config;
    config.loadFile(GameConfig::getDefaultConfigPath() / "openrw.ini");
    return config.getGameDataPath().string(); //FIXME: use path
}

tt::assertion_result with_data::operator()(utf::test_unit_id id) const {
    tt::assertion_result ans(global_args.with_data);
    if (!global_args.with_data) {
        auto& master = utf::framework::master_test_suite();
        ans.message() << "no data is available";
    }
    return ans;
}
