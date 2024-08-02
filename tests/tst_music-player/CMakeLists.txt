cmake_minimum_required(VERSION 3.10)

project(tst_music-player)

#set(QUICK_TEST_SOURCE_DIR "/media/feng/_dde_data/work/deepin-music/deepin-music/tests/tst_music-player")
#message("dir      " ${CMAKE_CURRENT_SOURCE_DIR} "     " ${QUICK_TEST_SOURCE_DIR})
#add_definitions( -DQUICK_TEST_SOURCE_DIR )


list(APPEND OTHER_FILES *.qml)

find_package(Dtk6Declarative REQUIRED)
find_package(Qt6 COMPONENTS Quick LinguistTools REQUIRED)
find_package(Qt6Test REQUIRED)

include_directories(../../src/music-player/allItems)
aux_source_directory(../../src/music-player/allItems ALLQMLSOURCE)

include_directories(.)

aux_source_directory(./ ALLSOURCE)

add_executable(${PROJECT_NAME} ${ALLSOURCE} ${RESOURCES} ${ALLQMLSOURCE} tst_test.qml)

# 添加gocv覆盖率文件的输出
# 内存检测
if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
    set(CMAKE_CXX_FLAGS "-g -fprofile-arcs -ftest-coverage")    #mips平台下不进行内存检测
else()
    set(CMAKE_CXX_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer -g -fprofile-arcs -ftest-coverage")
#    set(CMAKE_C_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
#    set(CMAKE_L_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
endif()

LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/../../src/libdmusic)
target_link_libraries(${PROJECT_NAME} dmusic)

file(COPY tst_test.qml DESTINATION ${PROJECT_BINARY_DIR})

