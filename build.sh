#!/bin/bash

DIR="build"
# add Release build type???
if cmake -DASSIMP_BUILD_TESTS=off -DASSIMP_NO_EXPORT=on -DCMAKE_BUILD_TYPE=Release -DGLFW_BUILD_DOCS=OFF -S . -B $DIR; then
	printf "\n"
	if cmake --build $DIR --parallel $(nproc); then
		exit 0
	else
		printf ">> build failed\n"
		exit 1
	fi
else
	printf ">> configure failed\n"
	exit 1
fi
