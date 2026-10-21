#!/bin/bash
export QT_QPA_PLATFORM='offscreen'
QTEST_FUNCTION_TIMEOUT='800000'
rm -rf ${HOME}/.cache/deepin/deepin-music/*
rm -rf ${HOME}/.config/deepin/deepin-music/*

export builddir=build-ut
export reportdir=build-ut
export scriptdir="$(cd "$(dirname "$0")" && pwd)"
export projectdir="$(cd "${scriptdir}/.." && pwd)"

# 项目根目录和构建目录
PROJECT_ROOT="${projectdir}"
BUILD_DIR="${projectdir}/${builddir}"
TESTS_DIR="${scriptdir}"

# 清理并创建构建目录
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"

# 使用 -S -B 参数明确指定源码和构建目录
cmake -DCMAKE_BUILD_TYPE=Debug -S "${PROJECT_ROOT}" -B "${BUILD_DIR}"
make -j8 -C "${BUILD_DIR}"

# 运行 libdmusic-test
cd "${BUILD_DIR}/tests/libdmusic-test"
set +e
bash "${TESTS_DIR}/libdmusic-test/ut-build-run.sh"
test_exit_code=$?
set -e

# 生成摘要 JSON
echo "==> Generating summary JSON: ${projectdir}/${reportdir}/ut-summary.json"

export GTEST_XML_DIR="${TESTS_DIR}/libdmusic-test/report"
export COVERAGE_INFO="${TESTS_DIR}/libdmusic-test/html/libdmusic-test_Coverage_fileter.info"
python3 "${scriptdir}/gen-ut-summary.py"

exit $test_exit_code
