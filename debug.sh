#!/bin/bash

DIR="debug"

if cmake -DASSIMP_BUILD_TESTS=off -DASSIMP_NO_EXPORT=on -DGLFW_BUILD_DOCS=OFF -DCMAKE_BUILD_TYPE=Debug -S . -B $DIR; then
	printf "\n"
	if cmake --build $DIR --parallel $(nproc); then
		printf "\n"
		exit 0
		# gdb -ex=r --args $@
	else
		printf ">> build failed\n"
	fi
else
	printf ">> configure failed\n"
fi
