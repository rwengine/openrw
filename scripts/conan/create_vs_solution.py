#!/usr/bin/env python3

import argparse
from pathlib import Path
import platform
import subprocess
import sys

from conans.client import conan_api


openrw_path = Path(__file__).resolve().parents[2]
cmake_generator_lookup = {
    2017: 'Visual Studio 15 2017',
}
architectures = ['x64', 'x86']

conan_arch_map = {
    'x86': 'x86',
    'x64': 'x86_64',
}


def to_cmake_generator(vs_version, arch):
    cmake_generator = cmake_generator_lookup[vs_version]
    if arch == 'x64':
        cmake_generator = '{} Win64'.format(cmake_generator)
    return cmake_generator
    
    
def create_solution(path, vs_version, arch):
    conan, _, _ = conan_api.ConanAPIV1.factory()
    conan.remote_add('bincrafters', 'https://api.bintray.com/conan/bincrafters/public-conan', force=True)
    conan_arch = conan_arch_map[arch]
    conan.install(path=openrw_path, generators=('cmake',), build=['missing', ],
                  settings=('build_type=Release', 'arch={}'.format(conan_arch), 'compiler.runtime=MD', ),
                  install_folder=path)
    cmake_generator = to_cmake_generator(vs_version=vs_version, arch=arch)
    subprocess.run([
        'cmake', '-DUSE_CONAN=ON', '-DBOOST_STATIC=ON', '-DMSVC_NO_DEBUG_RUNTIME=ON',
        '-DBUILD_TESTS=ON', '-DBUILD_VIEWER=ON', '-DBUILD_TOOLS=ON',
        '-G{}'.format(cmake_generator), str(openrw_path),
    ], cwd=path, check=True)


def main():
    parser = argparse.ArgumentParser(description='Create a Visual Studio solution for OpenRW.')
    parser.add_argument('path', nargs='?', default=Path(), metavar='PATH', type=Path, help='Location to the solution')
    parser.add_argument('-v', default=max(cmake_generator_lookup.keys()), choices=list(cmake_generator_lookup.keys()),
                        type=int, metavar='VERSION', dest='vs_version',
                        help='Version of Visual Studio (choices={})'.format(list(cmake_generator_lookup.keys())))
    parser.add_argument('-a', default=architectures[0], choices=architectures, metavar='ARCH', dest='arch',
                        help='Architecture to build (choices={})'.format(architectures))
    ns = parser.parse_args()

    if platform.system() != 'Windows':
        print('This script can only generate Visual Studio solutions for Windows.', file=sys.stderr)
        sys.exit(1)

    path = ns.path.resolve()
    arch = ns.arch
    vs_version = ns.vs_version

    print('Solution directory: {}'.format(path))
    print('Architecture: {}'.format(arch))
    print('Visual Studio version: {}'.format(vs_version))

    create_solution(path=path, vs_version=vs_version, arch=arch)


if __name__ == '__main__':
    main()
