#!/bin/bash

DIR="debug"

if cmake -DCMAKE_BUILD_TYPE=Debug -S . -B $DIR; then
	printf "\n"
	if cmake --build $DIR --parallel $(nproc); then
		printf "\n"
		exit 0
		# gdb -ex=r --args $@
	else
		printf ">> build failed\n"
		exit 1
	fi
else
	printf ">> configure failed\n"
	exit 1
fi
