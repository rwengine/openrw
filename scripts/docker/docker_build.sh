#!/bin/bash

curdir=$(dirname "$(readlink -f "$0")")

docker=$1

# Build docker image
"$curdir/docker_tool.py" build -d "$docker" -t openrw_build_image

# Start docker container + add travis user
"$curdir/docker_tool.py" create -t openrw_build_image -n openrw_builder -U travis \
    -e \
        ALSOFT_DRIVERS=null

# Build only
"$curdir/docker_tool.py" exec -n openrw_builder -U travis -- /bin/bash -c "cd /build && cmake -VV /src && make"
