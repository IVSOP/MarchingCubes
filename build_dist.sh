#!/bin/bash

DIR="build_dist"
LIB="$DIR/lib"

# add Release build type???
if cmake -DCMAKE_BUILD_TYPE=Release -DDistribution=on -S . -B $DIR; then
	printf "\n"
	if cmake --build $DIR --parallel $(nproc); then
		patchelf --set-rpath '$ORIGIN/lib' "$DIR/MarchingCubes"
		strip --strip-unneeded "$DIR/MarchingCubes"
		strip --strip-unneeded $LIB/*
		exit 0
	else
		printf ">> build failed\n"
		exit 1
	fi
else
	printf ">> configure failed\n"
	exit 1
fi
