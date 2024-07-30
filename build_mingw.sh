#!/bin/bash

DIR="buildWin"

if cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=TC-mingw.cmake -S . -B $DIR; then
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
