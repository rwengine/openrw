from conans import ConanFile, CMake


class OpenrwConan(ConanFile):
    name = 'openrw'
    version = 'master'
    license = 'GPL3'
    url = 'https://github.com/rwengine/openrw'
    description = "OpenRW 'Open ReWrite' is an un-official open source recreation of the classic Grand Theft Auto III game executable"
    settings = 'os', 'compiler', 'build_type', 'arch'
    options = {
        'viewer': [True, False],
        'tools': [True, False],
        'profiling': [True, False],
    }

    default_options = {
        'viewer': True,
        'tools': True,
        'profiling': True,
        'bullet3:shared': False,
        'sdl2:sdl2main': False,
        'ffmpeg:bzlib': False,
        'ffmpeg:lzma': False,
        'ffmpeg:openjpeg': False,
        'ffmpeg:openssl': False,
        'ffmpeg:vorbis': False,
        'ffmpeg:webp': False,
        'ffmpeg:x264': False,
        'ffmpeg:x265': False,
        'ffmpeg:zlib': False,
    }

    generators = 'cmake',
    exports_sources = 'CMakeLists.txt', 'cmake_configure.cmake', 'cmake_options.cmake', 'CMakeCPack.cmake', 'COPYING', \
                      'cmake/modules/*', 'benchmarks', 'rwcore/*', 'rwengine/*', 'rwgame/*', 'rwviewer/*', \
                      'rwtools/*', 'tests/*', 'external/*'

    _rw_dependencies = {
        'game': (
            'openal/1.19.1',
            'bullet3/2.87@bincrafters/stable',
            'glm/0.9.9.1@g-truc/stable',
            'ffmpeg/4.2.1@bincrafters/stable',
            'sdl2/2.0.9@bincrafters/stable',
            'boost/1.68.0@conan/stable',
            'bzip2/1.0.8',
            'zlib/1.2.11',
        ),
        'viewer': (
            'qt/5.14.0@bincrafters/stable',
            'glib/2.58.3@bincrafters/stable',
        ),
        'tools': (
            'freetype/2.10.1',
        ),
    }

    def configure(self):
        if self.options.viewer:
            self.options['qt'].opengl = 'desktop'

    def requirements(self):
        for dep in self._rw_dependencies['game']:
            self.requires(dep)
        if self.options.viewer:
            for dep in self._rw_dependencies['viewer']:
                self.requires(dep)
        if self.options.tools:
            for dep in self._rw_dependencies['tools']:
                self.requires(dep)

    def _configure_cmake(self):
        cmake = CMake(self)
        defs = {
            'BUILD_SHARED_LIBS': False,
            'CMAKE_BUILD_TYPE': self.settings.build_type,
            'BUILD_TESTS': True,
            'BUILD_VIEWER': self.options.viewer,
            'BUILD_TOOLS': self.options.tools,
            'ENABLE_PROFILING': self.options.profiling,
            'USE_CONAN': True,
            'BOOST_STATIC': not self.options['boost'].shared,
        }

        cmake.configure(defs=defs)
        return cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        if self.options.viewer:
            # FIXME: https://github.com/osechet/conan-qt/issues/6 and https://github.com/conan-io/conan/issues/2619
            self.copy('qt.conf', dst='bin', src='rwviewer')
        cmake = self._configure_cmake()
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ['rwengine', 'rwlib']
        self.cpp_info.stdcpp = 14
