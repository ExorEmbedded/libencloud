# Build from scratch and test making sure Qmake is called
#
# WARNING: cleans untracked files!!!

git clean -fdx .

qmake CONFIG+="endian modesece"

#qmake CONFIG+=modeece
#qmake CONFIG+=mode4ic

make check

if [ $? -ne 0 ]; then
    echo "Failure in build."
    exit 1
fi

exit 0
