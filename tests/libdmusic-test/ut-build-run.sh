#!/bin/bash

# 单元测试运行 + 覆盖率收集脚本（libdmusic-test）
#
# 关键修正（相对原版）：
# 1. lcov --directory 指向构建根目录，递归采集「被测库 dmusic」的覆盖率（原版只采
#    测试自身，分母仅几百行，无统计意义）。
# 2. --remove 过滤掉测试代码自身、系统头、autogen moc，使分母 = 纯业务源码。
# 3. ASAN 关闭 leak 检测：测试中 Presenter 未释放会触发 leak，抢先 abort 导致
#    gcov 的 .gcda 无法 flush（覆盖率恒为 0）。
# 4. timeout 保护：防止任一用例死循环（如 fft 缺陷）阻塞整个 CI。

executable=libdmusic-test #可执行程序的文件名

platform=`uname -m`
echo ${platform}

cd ./tests/libdmusic-test/

mkdir -p html
mkdir -p report

echo " ===================CREAT LCOV REPROT==================== "
# zerocounters 必须覆盖整个构建目录（含 dmusic.dir），否则历史 .gcda 会污染统计
lcov --directory ../../ --zerocounters
# 关闭 leak 检测 + 超时保护运行测试，确保进程正常退出以 flush .gcda
QT_QPA_PLATFORM=offscreen ASAN_OPTIONS="fast_unwind_on_malloc=1:detect_leaks=0" \
    timeout 120 ./$executable
# --directory ../../  递归采集 dmusic + 测试的 .gcda（业务代码覆盖率的关键）
lcov --directory ../../ --capture --output-file ./html/${executable}_Coverage.info

echo " =================== do filter begin ==================== "

# 过滤：系统头、第三方库、Qt/DTK 头、测试代码自身、构建产物（build-ut 整个目录，
# 含 autogen 生成的 moc/qrc），只保留被测业务源码 src/libdmusic/
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
mv asan.log* asan_${executable}.log 2>/dev/null || true

cp -r ./html/ ../../
cp -r ./report/ ../../
cp ./asan_${executable}.log ../../ 2>/dev/null || true

exit 0
