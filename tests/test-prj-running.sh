#!/bin/bash
export QT_QPA_PLATFORM='offscreen'
QTEST_FUNCTION_TIMEOUT='800000'
rm -rf ${HOME}/.cache/deepin/deepin-music/*
rm -rf build
mkdir build
cd build
cp ../../tests/collection-coverage.sh ./
cmake ..
make -j8

lcov --directory ./CMakeFiles/deepin-music-test.dir --zerocounters
./build/music-player/deepin-music-test
#第一次会因为未知原因失败
./collection-coverage.sh
sleep 5
#第一次因为未知原因失败,执行第二次进行收集
./collection-coverage.sh
#make test -j8

cd ./../../
rm -rf ./build-ut
mkdir build-ut
cd build-ut

cp -r ./../tests/build/coverageResult/report/ ./
mv report html
cd html
mv index.html cov_deepin-music.html

cd ..
mkdir report
cd report
cp ./../../tests/build/report/report_deepin-music.xml ./

cd ..
cp ./../tests/build/asan_deepin-music.log* ./asan_deepin-music.log

exit 0
