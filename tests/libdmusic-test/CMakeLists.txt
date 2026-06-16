cmake_minimum_required(VERSION 3.10)

project(libdmusic-test)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

#ADD_DEFINITIONS( -LITE_DIV)
execute_process(COMMAND uname -m OUTPUT_VARIABLE MACH
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DMR_DEBUG on)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)
set(OBJECT_DIR ${CMAKE_BINARY_DIR}/CMakeFiles/objects)


# 设置Qt模块（对齐主工程 Qt6 + Core5Compat）
set(QT_MODULES Core Gui Widgets Svg Multimedia Xml Network Sql DBus Test Core5Compat)

#先查找到这些qt相关的模块以供链接使用
find_package(Qt6 REQUIRED COMPONENTS ${QT_MODULES})

# 包含源码文件夹
include_directories(${CMAKE_INCLUDE_CURRENT_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(../../src/libdmusic)
include_directories(../../src/libdmusic/core)
include_directories(../../src/libdmusic/player)
include_directories(../../src/libdmusic/player/vlc)
include_directories(../../src/libdmusic/util)

FILE (GLOB allTestSource
    *.cpp
#    *.sh
    )
# 定义资源文件
#set(AllQRC
#    ../../assets/resources.qrc
#    ../../assets/icons/theme-icons.qrc
#)

LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/src/libdmusic)

# 设置不删除生成的文件夹内容文件（设置此项是为了翻译后的文件不被删除）
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

# 添加需要安装的模块
add_executable(${PROJECT_NAME} ${allTestSource})

# 测试数据目录宏（testdata/sample.mp3 等真实音频样本，编译期注入源码绝对路径）
target_compile_definitions(${PROJECT_NAME} PRIVATE TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/testdata")

# 添加gocv覆盖率文件的输出
# 内存检测
if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
    set(CMAKE_CXX_FLAGS "-g -fprofile-arcs -ftest-coverage")    #mips平台下不进行内存检测
else()
    set(CMAKE_CXX_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer -g -fprofile-arcs -ftest-coverage")
#    set(CMAKE_C_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
#    set(CMAKE_L_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
endif()

find_package(PkgConfig REQUIRED)

# 对齐主工程依赖集：mpris-qt6、taglib、dtk6core；移除主工程已废弃的 udisks2-qt5 与 KF5Codecs
pkg_check_modules(MPRIS REQUIRED IMPORTED_TARGET mpris-qt6)
pkg_check_modules(TAGLIB REQUIRED IMPORTED_TARGET taglib)
pkg_check_modules(DTK REQUIRED IMPORTED_TARGET dtk6core)

LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/../../src/libdmusic)
set(TARGET_LIBS PkgConfig::MPRIS PkgConfig::TAGLIB PkgConfig::DTK Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Svg Qt6::Multimedia Qt6::Xml Qt6::Network Qt6::Sql Qt6::DBus Qt6::Core5Compat)
target_include_directories(${PROJECT_NAME} PUBLIC ${TARGET_LIBS} Qt6::Test)
target_link_libraries(${PROJECT_NAME} ${TARGET_LIBS} Qt6::Test dmusic)
target_link_libraries(${PROJECT_NAME} gmock gmock_main gtest gtest_main -lpthread -lm)

include(GNUInstallDirs)

