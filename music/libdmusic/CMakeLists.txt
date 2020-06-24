#工程名
project(dmusic)
cmake_minimum_required(VERSION 3.10)

#文件路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/dist/lib)

#文件夹包含
include_directories(.)
include_directories(net)
include_directories(util)

aux_source_directory(. allSourcelib)
aux_source_directory(net allSourcelib)
aux_source_directory(util allSourcelib)

add_library(dmusic SHARED ${allSourcelib})

#使用第三方库需要用到的一个包
find_package(PkgConfig REQUIRED)

#检查第三方库(这里检查了名字为dtkwidget的库和名字为dtkgui的库)，然后取名3rd_lib
pkg_check_modules(3rd_lib REQUIRED
        dtkwidget dtkgui
        )
#添加第三方库的所有文件夹路径到工程中来(注意 *_INCLUDE_DIRS)
target_include_directories(dmusic PUBLIC ${3rd_lib_INCLUDE_DIRS})

#将第三方库链接进来(注意 *_LIBRARIES)
target_link_libraries(dmusic ${3rd_lib_LIBRARIES})

qt5_use_modules(dmusic ${QtModule})

#单独查找KCodecs包
find_package(KF5Codecs)
target_link_libraries(dmusic KF5::Codecs)

