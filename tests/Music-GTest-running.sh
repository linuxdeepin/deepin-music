#!/bin/bash
currentdir=$(dirname $(readlink -f "$0"))
cd $currentdir/build/CMakeFiles/deepin-music-test.dir/

lcov -d $currentdir -c -o $currentdir/music-coverage.info

remove_info="*/usr/include/*"
remove_info_compilcer="*$currentdir/build/deepin-music-test_autogen/*"
remove_info_gtest="*/home/tx-zy/work/subtest/bug-fix/tests/*"
remove_info_mpris="*$currentdir/build/vendor/mpris-qt/src/mpris-qt5_autogen/*" 
remove_info_dbusextended="*$currentdir/build/vendor/dbusextended-qt/src/dbusextended-qt5_autogen/*"
remove_info_libdmusic="*$currentdir/build/libdmusic/dmusic_autogen/*"
remove_info_qrc="*$currentdir/build/*"
result_coverage_dir=$currentdir/coverage

lcov --remove $currentdir/music-coverage.info $remove_info $remove_info_compilcer $remove_info_gtest $remove_info_mpris $remove_info_dbusextended $remove_info_libdmusic $remove_info_qrc --output-file $currentdir/coverage.info

genhtml -o $result_coverage_dir $currentdir/coverage.info

nohup x-www-browser $result_coverage_dir/index.html &

lcov -d $currentdir -z

exit 0

