#!/bin/bash

DIR="build"
# add Release build type???
if cmake -DCMAKE_BUILD_TYPE=Release -S . -B $DIR; then
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
