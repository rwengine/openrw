from conans import ConanFile, CMake
from conans.errors import ConanException


class OpenrwConan(ConanFile):
    name = 'openrw'
    version = 'master'
    license = 'GPL3'
    url = 'https://github.com/rwengine/openrw'
    description = "OpenRW 'Open ReWrite' is an un-official open source recreation of the classic Grand Theft Auto III game executable"
    settings = 'os', 'compiler', 'build_type', 'arch'
    options = {
        'test_data': [True, False],
        'viewer': [True, False],
        'tools': [True, False],
    }

    default_options = (
        'test_data=False',
        'viewer=True',
        'tools=True',
        'bullet:shared=False',
        'sdl2:sdl2main=False',
    )

    generators = 'cmake',
    exports_sources = 'CMakeLists.txt', 'cmake_configure.cmake', 'cmake_options.cmake', 'COPYING', \
                      'cmake/modules/*', 'benchmarks', 'rwlib/*', 'rwengine/*', 'rwgame/*', 'rwviewer/*', 'tests/*'

    _rw_dependencies = {
        'game': (
            'openal/1.18.2@bincrafters/stable',
            'bullet/2.87@bincrafters/stable',
            'glm/0.9.9.1@g-truc/stable',
            'ffmpeg/4.0@bincrafters/stable',
            'sdl2/2.0.8@bincrafters/stable',
            'boost/1.67.0@conan/stable',
        ),
        'viewer': (
            'Qt/5.11.1@bincrafters/stable',
        ),
        'tools': (
            'freetype/2.9.0@bincrafters/stable',
        ),
    }

    def configure(self):
        if self.options.viewer:
            self.options['Qt'].opengl = 'desktop'

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
            'TESTS_NODATA': not self.options.test_data,
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
