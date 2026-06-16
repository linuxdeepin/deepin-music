#!/bin/bash

#workdir=$(cd ../$(dirname $0)/deepin-movie/build-ut; pwd)
executable=tst_music-player #可执行程序的文件名

platform=`uname -m`
echo ${platform}

cd ./tests/tst_music-player/

mkdir -p html
mkdir -p report

echo " ===================CREAT LCOV REPROT==================== "
lcov --directory ./CMakeFiles/tst_music-player.dir --zerocounters
ASAN_OPTIONS="fast_unwind_on_malloc=1" ./$executable
lcov --directory . --capture --output-file ./html/${executable}_Coverage.info

echo " =================== do filter begin ==================== "

# 过滤：系统头、测试代码自身、构建产物（build-ut 目录含 autogen 生成的 moc/qrc）
lcov --remove ./html/${executable}_Coverage.info \
    '*/usr/include/*' \
    '/usr/local/*' \
    '*/tests/*' \
    '*/build-ut/*' \
    '*/build*/*_autogen/*' \
    '*/moc_*.cpp' \
    '*/qrc_*.cpp' \
    -o ./html/${executable}_Coverage_fileter.info

echo " =================== do filter end ====================== "
    
genhtml -o ./html ./html/${executable}_Coverage_fileter.info
    
mv ./html/index.html ./html/cov_${executable}.html
mv asan.log* asan_${executable}.log

cp -r ./html/ ../../
cp -r ./report/ ../../
cp ./asan_${executable}.log ../../

#ls report/

exit 0
