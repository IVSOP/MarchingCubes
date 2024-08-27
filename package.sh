#!/usr/bin/bash
# am bad at names


#TODO:
# distribution build, with IPO and other options
# strip all symbols (see how its done in the AUR?)


echo "NOT BUILDING ANYTHING!!!!!!!!!!"

if [ "$#" -ne 1 ]
then
    echo "Specify linux or windows"
    exit 1
fi

if [ "$1" == "linux" ]
then
	echo "Not done"
else
    if [ "$1" == "windows" ]
    then
		# ugly hack, * is done by the shell and no *.dll in current context, need to move into build folder
		dlls=$(cd buildWin; echo *.dll)
		tar --owner=0 --group=0 --no-same-owner --no-same-permissions -c steam_appid.txt --files-from <(grep -Ev '^(\#|\/\/).*' files.txt) -C buildWin MarchingCubes.exe $dlls -f - | zstd -10 --long --threads=0 --stdout > MarchingCubes.tar.zst
    else
        echo "Specify linux or windows"
    fi
fi

