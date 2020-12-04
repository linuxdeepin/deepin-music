#工程名
#project(dmusic)
set(CMD_NAME dmusic)
cmake_minimum_required(VERSION 3.10)

#文件路径
#set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../dist/lib)

#文件夹包含
include_directories(.)
#include_directories(net)
include_directories(util)

aux_source_directory(. allSourcelib)
#aux_source_directory(net allSourcelib)
aux_source_directory(util allSourcelib)

add_library(${CMD_NAME} STATIC ${allSourcelib})
set_target_properties(dmusic PROPERTIES VERSION 1.0.0 SOVERSION 1.0)

#使用第三方库需要用到的一个包
find_package(PkgConfig REQUIRED)

#检查第三方库(这里检查了名字为dtkwidget的库和名字为dtkgui的库)，然后取名3rd_lib
pkg_check_modules(3rd_lib REQUIRED
        dtkwidget dtkgui dtkcore
        )
#添加第三方库的所有文件夹路径到工程中来(注意 *_INCLUDE_DIRS)
target_include_directories(${CMD_NAME} PUBLIC ${3rd_lib_INCLUDE_DIRS})

#将第三方库链接进来(注意 *_LIBRARIES)
target_link_libraries(${CMD_NAME} icui18n)
target_link_libraries(${CMD_NAME} ${3rd_lib_LIBRARIES})

qt5_use_modules(${CMD_NAME} ${QtModule})

#单独查找KCodecs包
find_package(KF5Codecs)
target_link_libraries(${CMD_NAME} KF5::Codecs)
install(TARGETS ${CMD_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
