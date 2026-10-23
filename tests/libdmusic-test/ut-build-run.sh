#!/bin/bash

# 单元测试运行 + 覆盖率收集脚本（libdmusic-test）

executable=libdmusic-test

platform=`uname -m`
echo ${platform}

# 获取测试可执行文件目录（脚本所在目录）
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)/build-ut"

mkdir -p "${SCRIPT_DIR}/html"
mkdir -p "${SCRIPT_DIR}/report"

echo " ===================CREAT LCOV REPROT==================== "
lcov --directory "${BUILD_DIR}" --zerocounters

# 运行测试（GTest XML 输出路径在 test_main.cpp 中硬编码为 ./report/）
# 从源码目录运行，使 ./report/ 解析到 SCRIPT_DIR/report
set +e
cd "${SCRIPT_DIR}"
QT_QPA_PLATFORM=offscreen UBSAN_OPTIONS="halt_on_error=0:print_stacktrace=1" \
    timeout 120 "${BUILD_DIR}/tests/libdmusic-test/${executable}"
test_exit_code=$?
set -e

lcov --directory "${BUILD_DIR}" --capture --output-file "${SCRIPT_DIR}/html/${executable}_Coverage.info"

echo " =================== do filter begin ==================== "

lcov --remove "${SCRIPT_DIR}/html/${executable}_Coverage.info" \
    '*/usr/include/*' \
    '/usr/local/*' \
    '*/tests/*' \
    '*/build-ut/*' \
    '*/build*/*_autogen/*' \
    '*/moc_*.cpp' \
    '*/qrc_*.cpp' \
    -o "${SCRIPT_DIR}/html/${executable}_Coverage_fileter.info"

echo " =================== do filter end ====================== "

genhtml -o "${SCRIPT_DIR}/html" "${SCRIPT_DIR}/html/${executable}_Coverage_fileter.info"

mv "${SCRIPT_DIR}/html/index.html" "${SCRIPT_DIR}/html/cov_${executable}.html"

exit $test_exit_code
