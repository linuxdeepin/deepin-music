#!/bin/bash

#workdir=$(cd ../$(dirname $0)/deepin-movie/build-ut; pwd)
executable=libdmusic-test #可执行程序的文件名

platform=`uname -m`
echo ${platform}

cd ./tests/libdmusic-test/

mkdir -p html
mkdir -p report

echo " ===================CREAT LCOV REPROT==================== "
lcov --directory ./CMakeFiles/libdmusic-test.dir --zerocounters
ASAN_OPTIONS="fast_unwind_on_malloc=1" ./$executable
lcov --directory . --capture --output-file ./html/${executable}_Coverage.info

echo " =================== do filter begin ==================== "

lcov --remove ./html/${executable}_Coverage.info '*/usr/include/*' '/usr/local/*'  -o ./html/${executable}_Coverage_fileter.info

echo " =================== do filter end ====================== "
    
genhtml -o ./html ./html/${executable}_Coverage_fileter.info
    
mv ./html/index.html ./html/cov_${executable}.html
mv asan.log* asan_${executable}.log

cp -r ./html/ ../../
cp -r ./report/ ../../
cp ./asan_${executable}.log ../../

#ls report/

exit 0
