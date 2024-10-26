#!/usr/bin/bash
# am bad at names

echo "NOT BUILDING ANYTHING!!!!!!!!!! build it yourself"

if [ "$#" -ne 1 ]
then
    echo "Specify linux or windows"
    exit 1
fi

if [ "$1" == "linux" ]
then
	# LIBS=$(cd build_dist/lib; echo *)
	tar --owner=0 --group=0 --no-same-owner --no-same-permissions -c steam_appid.txt --files-from <(grep -Ev '^(\#|\/\/).*' files.txt) -C build_dist MarchingCubes lib/ -f - | zstd -10 --long --threads=0 --stdout > MarchingCubes.tar.zst
else
    if [ "$1" == "windows" ]
    then
		LIBS=$(cd buildWin/lib; echo *)
		tar --owner=0 --group=0 --no-same-owner --no-same-permissions -c steam_appid.txt --files-from <(grep -Ev '^(\#|\/\/).*' files.txt) -C buildWin MarchingCubes.exe -C lib $LIBS -f - | zstd -10 --long --threads=0 --stdout > MarchingCubes_windows.tar.zst
	else
        echo "Specify linux or windows"
    fi
fi

