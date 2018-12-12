// RWConfig: Category: WINDOW, INPUT, DEVELOP, GAME, GENERAL

// RWCONFIGARG: option available in argument parser and configuration file: ALWAYS std::optional
// RWARG_OPT: option only available in argument parser: ALWAYS std::optional
// RWARG: option only available in argument parser: NEVER std::optional

RWCONFIGARG(std::string,    gamedataPath,   std::nullopt,           "game.path",            CONFIG,     "gamedata",     "PATH",     "Path of gamedata")
RWARG_OPT(  std::string,    configPath,                                                     CONFIG,     "config,c",     "PATH",     "Path of configuration file")
RWARG(      bool,           noconfig,                                                       CONFIG,     "noconfig",     nullptr,    "Don't load configuration file")

RWCONFIGARG(bool,           invertY,        false,                  "input.invert_y",       INPUT,      "invert_y",     nullptr,    "Invert the y-axis of the mouse")

RWCONFIGARG(int,            width,          800,                    "window.width",         WINDOW,     "width,w",      "WIDTH",    "Game resolution width in pixels")
RWCONFIGARG(int,            height,         600,                    "window.height",        WINDOW,     "height,h",     "HEIGHT",   "Game resolution height in pixels")
RWCONFIGARG(bool,           fullscreen,     false,                  "window.fullscreen",    WINDOW,     "fullscreen,f", nullptr,    "Enable fullscreen mode")
RWCONFIGARG(float,          hudScale,       1.f,                    "game.hud_scale",       WINDOW,     "hud_scale",    "FACTOR",   "Scaling factor of the HUD")

RWARG(      bool,           test,                                                           DEVELOP,    "test,t",       nullptr,    "Start a new game in a test location")
RWARG_OPT(  std::string,    benchmarkPath,                                                  DEVELOP,    "benchmark,b",  "PATH",     "Run benchmark from file")

RWARG(      bool,           newGame,                                                        GAME,       "newgame,n",    nullptr,    "Start a new game")
RWARG_OPT(  std::string,    loadGamePath,                                                   GAME,       "load,l",       "PATH",     "Load save file")
RWCONFIGARG(std::string,    gameLanguage,   "american",             "game.language",        GAME,       "language",     "LANGUAGE", "Language")

RWARG(      bool,           help,                                                           GENERAL,    "help",         nullptr,    "Show this help message")
