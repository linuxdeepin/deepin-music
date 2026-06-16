#!/bin/bash
export QT_QPA_PLATFORM='offscreen'
QTEST_FUNCTION_TIMEOUT='800000'
rm -rf ${HOME}/.cache/deepin/deepin-music/*
rm -rf ${HOME}/.config/deepin/deepin-music/*

# 项目根目录和构建目录
PROJECT_ROOT="$(cd .. && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build-ut"
TESTS_DIR="${PROJECT_ROOT}/tests"

# 清理并创建构建目录
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"

# 使用 -S -B 参数明确指定源码和构建目录
cmake -DCMAKE_BUILD_TYPE=Debug -S "${PROJECT_ROOT}" -B "${BUILD_DIR}"
make -j8 -C "${BUILD_DIR}"

# 运行 libdmusic-test
cd "${BUILD_DIR}/tests/libdmusic-test"
bash "${TESTS_DIR}/libdmusic-test/ut-build-run.sh"

exit 0
