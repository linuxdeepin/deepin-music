# tst_music-player 仅为 2+2=4 桩测试（含一个故意失败的用例 test_fail），
# 且其 CMake 将 .qml 作为源传入 add_executable、依赖 Dtk6Declarative，构建即失败。
# 本次 Qt6 迁移聚焦 libdmusic-test（真正的后端单测）；QML UI 测试待后续按真实用例重建。
# add_subdirectory(tst_music-player)
add_subdirectory(libdmusic-test)

# ── 模块化单元测试自动发现（增量补全）─────────────────────────────────────
# 每个测试类独立子目录 tests/ut_<classname>/，含独立 CMakeLists.txt。
# 新增测试只需新建 ut_<classname>/ 子目录，无需修改本文件，避免多 PR 合并冲突。
file(GLOB MULTICA_UT_SUBDIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/ut_*)
foreach(_ut_dir ${MULTICA_UT_SUBDIRS})
    add_subdirectory(${_ut_dir})
endforeach()


