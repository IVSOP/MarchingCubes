#!/usr/bin/bash
# am bad at names

if [ "$#" -ne 1 ]
then
    echo "Specify linux or windows"
    exit 1
fi

if [ "$1" == "linux" ]
then
    echo "Unfinished"
else
    if [ "$1" == "windows" ]
    then
	tar -c steam_appid.txt buildWin/MarchingCubes.exe buildWin/glew32.dll buildWin/steam_api64.dll shaders/ textures/ -f - | zstd -10 --long --threads=0 --stdout > MarchingCubes.tar.zst
    else
        echo "Specify linux or windows"
    fi
fi

