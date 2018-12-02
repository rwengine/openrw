#!/bin/bash

curdir=$(dirname "$(readlink -f "$0")")

docker=$1

# Build docker image
"$curdir/docker_tool.py" build -d "$docker" -t openrw_build_image

# Start docker container + add travis user
"$curdir/docker_tool.py" create -t openrw_build_image -n openrw_builder -U travis \
    -e \
        NAME_SUFFIX=$NAME_SUFFIX \
        CI=$CI \
        TRAVIS=$TRAVIS \
        SHIPPABLE=$SHIPPABLE \
        CODECOV_ENV=$CODECOV_ENV \
        CODECOV_TOKEN=$CODECOV_TOKEN \
        CODECOV_URL=$CODECOV_URL \
        CODECOV_SLUG=$CODECOV_SLUG \
        TRAVIS_REPO_SLUG=$TRAVIS_REPO_SLUG \
        TRAVIS_BRANCH=$TRAVIS_BRANCH \
        TRAVIS_COMMIT=$TRAVIS_COMMIT \
        TRAVIS_JOB_NUMBER=$TRAVIS_JOB_NUMBER \
        TRAVIS_PULL_REQUEST=$TRAVIS_PULL_REQUEST \
        TRAVIS_JOB_ID=$TRAVIS_JOB_ID \
        TRAVIS_TAG=$TRAVIS_TAG \
        TRAVIS_OS_NAME=$TRAVIS_OS_NAME \
        TRAVIS_EVENT_TYPE=$TRAVIS_EVENT_TYPE \
        USE_CONAN=$USE_CONAN \
        TEST_COVERAGE=$TEST_COVERAGE \
        ALSOFT_DRIVERS=null \
        CC=$CC \
        CXX=$CXX \
        DEBUG=$DEBUG \
        XDG_RUNTIME_DIR=/tmp

# execute test
"$curdir/docker_tool.py" exec -n openrw_builder -U travis -- /bin/bash -c \
    "ctest -VV -S /src/cmake/ctest/script_ci.ctest"
