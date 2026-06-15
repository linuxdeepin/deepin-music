# tst_music-player 仅为 2+2=4 桩测试（含一个故意失败的用例 test_fail），
# 且其 CMake 将 .qml 作为源传入 add_executable、依赖 Dtk6Declarative，构建即失败。
# 本次 Qt6 迁移聚焦 libdmusic-test（真正的后端单测）；QML UI 测试待后续按真实用例重建。
# add_subdirectory(tst_music-player)
add_subdirectory(libdmusic-test)


