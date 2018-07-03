#!/usr/bin/env python3

import argparse
import os
from pathlib import Path
import pwd
import subprocess

docker_dir = Path(__file__).absolute().parent
docker_files = list(docker_dir.glob('*.docker'))
openrw_dir = docker_dir.parents[1]
openrw_build_dir = openrw_dir / 'build'
assert((openrw_dir/'CMakeLists.txt').exists())


def sub_run(args):
    print('Executing', args)
    subprocess.check_call(args)


def build(ns: argparse.Namespace):
    docker_file = docker_dir / ns.distribution
    assert(docker_file.exists())
    sub_run(['docker', 'build', '-t', ns.tag, '-f', str(docker_file), str(openrw_dir)])


def create(ns: argparse.Namespace):
    # name: str, tag: str, build_subdir: str, uid: typing.Union[None, int], username: str):
    build_dir = openrw_build_dir / ns.build_subdir
    if not build_dir.exists():
        build_dir.mkdir()
    env_args = [a for e in ns.env for a in ('-e', e)]
    env_args += ['-e', 'DOCKER=TRUE', ]
    run_args = ['docker', 'run',
                    '-v', '{}:/src:ro,z'.format(str(openrw_dir)),
                    '-v', '{}:/build:rw,z'.format(str(build_dir)) ] + \
               env_args + ['--name', ns.name, '-d', ns.tag, 'sleep', 'infinity']
    sub_run(run_args)
    if ns.uid is None:
        ns.uid = os.getuid()
    gid = pwd.getpwuid(ns.uid).pw_gid
    sub_run(['docker', 'exec', ns.name, 'groupadd', '--gid', str(gid), ns.username])
    sub_run(['docker', 'exec', ns.name, 'useradd', '--create-home',
                    '--uid', str(ns.uid), '-g', ns.username, ns.username])
    sub_run(['docker', 'exec', ns.name, 'chown', '{0}:{0}'.format(ns.username),
                    '/build'])
    sub_run(['docker', 'exec', ns.name, 'usermod', '-a', '-G', 'sudo', ns.username])
    sub_run(['docker', 'exec', ns.name, '/bin/bash', '-c', 'printf "{0}:{0}" | chpasswd'.format(ns.username)])
    sub_run(['docker', 'exec', ns.name, '/bin/bash', '-c', 'printf "{0} ALL= NOPASSWD: ALL\n" >> /etc/sudoers'.format(ns.username)])


def exec(ns: argparse.Namespace):
    if ns.ARGUMENTS:
        run_args = ['docker', 'exec', '-u', ns.username, ns.name ] + ns.ARGUMENTS
    else:
        run_args = ['docker', 'exec', '-ti', '-u', ns.username, ns.name, '/bin/bash']
    sub_run(run_args)


def main():
    parser = argparse.ArgumentParser()
    subparser = parser.add_subparsers()

    build_parser = subparser.add_parser('build')
    build_parser.add_argument('-d', '--distribution', required=True, choices=tuple(df.name for df in docker_files),
                              help='which Docker image to build')
    build_parser.add_argument('-t', '--tag', required=True, help='Docker image name')
    build_parser.set_defaults(func=build)

    create_parser = subparser.add_parser('create')
    create_parser.add_argument('-e', '--env', default=[], nargs=argparse.ONE_OR_MORE, help='Add environment variables')
    create_parser.add_argument('-n', '--name', required=True, help='Name of the container')
    create_parser.add_argument('-b', '--build_subdir', default='.', help='/build subdir')
    create_parser.add_argument('-t', '--tag', required=True, help='Docker image name')
    create_parser.add_argument('-u', '--uid', type=int, default=None, help='uid of the user to add to the Docker image')
    create_parser.add_argument('-U', '--username', type=str, required=True, help='name of the user to add to the Docker image')
    create_parser.set_defaults(func=create)

    exec_parser = subparser.add_parser('exec')
    exec_parser.add_argument('ARGUMENTS', nargs=argparse.ZERO_OR_MORE)
    exec_parser.add_argument('-n', '--name', required=True, help='Name of the container')
    exec_parser.add_argument('-U', '--username', type=str, required=True, help='username to execute commands as')
    exec_parser.set_defaults(func=exec)

    ns = parser.parse_args()
    if not hasattr(ns, 'func'):
        parser.error('Need an action to perform')
    ns.func(ns)

if __name__ == '__main__':
    main()
