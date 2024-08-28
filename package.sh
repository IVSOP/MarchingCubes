#!/usr/bin/bash
# am bad at names

echo "NOT BUILDING ANYTHING!!!!!!!!!!"

if [ "$#" -ne 1 ]
then
    echo "Specify linux or windows"
    exit 1
fi

if [ "$1" == "linux" ]
then
	sos=($cd build_dist; echo *.so)
	tar --owner=0 --group=0 --no-same-owner --no-same-permissions -c steam_appid.txt --files-from <(grep -Ev '^(\#|\/\/).*' files.txt) -C build_dist MarchingCubes $sos -f - | zstd -10 --long --threads=0 --stdout > MarchingCubes.tar.zst
else
    if [ "$1" == "windows" ]
    then
		# ugly hack, * is done by the shell and no *.dll in current context, need to move into build folder
		dlls=$(cd buildWin; echo *.dll)
		tar --owner=0 --group=0 --no-same-owner --no-same-permissions -c steam_appid.txt --files-from <(grep -Ev '^(\#|\/\/).*' files.txt) -C buildWin MarchingCubes.exe $dlls -f - | zstd -10 --long --threads=0 --stdout > MarchingCubes_windows.tar.zst
	else
        echo "Specify linux or windows"
    fi
fi

