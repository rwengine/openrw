#!/bin/bash

curdir=$(dirname "$(readlink -f "$0")")

docker=$1

# Build docker image
"$curdir/docker_tool.py" build -d "$docker" -t openrw_build_image

# Start docker container + add travis user
"$curdir/docker_tool.py" create -t openrw_build_image -n openrw_builder -U travis \
    -e \
        NAME_SUFFIX=$NAME_SUFFIX \
        TRAVIS=$TRAVIS \
        TRAVIS_COMMIT=$TRAVIS_COMMIT \
        TRAVIS_EVENT_TYPE=$TRAVIS_EVENT_TYPE \
        TRAVIS_REPO_SLUG=$TRAVIS_REPO_SLUG \
        TRAVIS_BRANCH=$TRAVIS_BRANCH \
        USE_CONAN=$USE_CONAN \
        ALSOFT_DRIVERS=null \
        DEBUG=$DEBUG \
        XDG_RUNTIME_DIR=/tmp

# execute test
"$curdir/docker_tool.py" exec -n openrw_builder -U travis -- /bin/bash -c "ctest -VV -S /src/cmake/ctest/script_ci.ctest -VV"
