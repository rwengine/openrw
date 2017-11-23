#!/bin/bash

curdir=$(dirname "$(readlink -f "$0")")

docker=$1

# Build docker image
"$curdir/docker_tool.py" build -d "$docker" -t openrw_build_image

# Start docker container + add travis user
"$curdir/docker_tool.py" create -t openrw_build_image -n openrw_builder -U travis -e XDG_RUNTIME_DIR=/tmp CI=$CI TRAVIS=$TRAVIS TRAVIS_COMMIT=$TRAVIS_COMMIT TRAVIS_EVENT_TYPE=$TRAVIS_EVENT_TYPE TRAVIS_REPO_SLUG=$TRAVIS_REPO_SLUG TRAVIS_BRANCH=$TRAVIS_BRANCH

# execute test
"$curdir/docker_tool.py" exec -n openrw_builder -U travis -- /bin/bash -c "ctest -S /src/cmake/ctest/travis_script.ctest -VV -DSRC_DIR=/src -DBIN_DIR=/build"
