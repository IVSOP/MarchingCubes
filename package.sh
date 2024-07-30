#!/usr/bin/bash
# am bad at names

if [ "$#" -ne 1 ]
then
    echo "Specify linux or windows"
    exit 1
fi

if [ "$1" == "linux" ]
then
   	tar --owner=0 --group=0 --no-same-owner --no-same-permissions -c steam_appid.txt shaders/ textures/ -C build MarchingCubes *.so -f - | zstd -10 --long --threads=0 --stdout > MarchingCubes.tar.zst
else
    if [ "$1" == "windows" ]
    then
	tar --owner=0 --group=0 --no-same-owner --no-same-permissions -c steam_appid.txt shaders/ textures/ assets/ -C buildWin MarchingCubes.exe *.dll -f - | zstd -10 --long --threads=0 --stdout > MarchingCubes.tar.zst
    else
        echo "Specify linux or windows"
    fi
fi

