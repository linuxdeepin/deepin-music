#!/bin/bash
export QT_QPA_PLATFORM='offscreen'
QTEST_FUNCTION_TIMEOUT='800000'
rm -rf ${HOME}/.cache/deepin/deepin-music/*
rm -rf ${HOME}/.config/deepin/deepin-music/*
rm -rf ../$(dirname $0)/build-ut
mkdir ../$(dirname $0)/build-ut
cd ../build-ut
#cp ../../tests/collection-coverage.sh ./
cmake -DCMAKE_BUILD_TYPE=Debug ../
make -j8


../tests/libdmusic-test/ut-build-run.sh
../tests/tst_music-player/ut-build-run.sh

exit 0
