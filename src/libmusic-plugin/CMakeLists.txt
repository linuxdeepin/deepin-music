#工程名
set(CMD_NAME music-plugin)
set(CMAKE_INSTALL_VOICE_LIBDIR /usr/lib/deepin-aiassistant/serivce-plugins)
cmake_minimum_required(VERSION 3.10)

#文件夹包含
include_directories(.)
include_directories(./interface)

aux_source_directory(. allSourcelib)

file(GLOB_RECURSE QRC_SOURCE_FILES ./*.qrc)
set(QRC_FILES config.qrc)
qt5_add_resources(QRC_FILES ${QRC_SOURCE_FILES})

add_library(${CMD_NAME} SHARED ${allSourcelib} ${QRC_FILES} interface/service.h)

#使用第三方库需要用到的一个包
find_package(PkgConfig REQUIRED)
pkg_check_modules(Dtk REQUIRED IMPORTED_TARGET dtkcore)
#添加第三方库的所有文件夹路径到工程中来(注意 *_INCLUDE_DIRS)
target_include_directories(${CMD_NAME} PUBLIC ${Dtk_INCLUDE_DIRS})

#检查第三方库(这里检查了名字为dtkwidget的库和名字为dtkgui的库)，然后取名3rd_lib
pkg_check_modules(3rd_lib REQUIRED  dframeworkdbus dtkcore)
#将第三方库链接进来(注意 *_LIBRARIES)
target_link_libraries(${CMD_NAME} ${3rd_lib_LIBRARIES})

qt5_use_modules(${CMD_NAME} ${QtModule})

install(TARGETS ${CMD_NAME} DESTINATION ${CMAKE_INSTALL_VOICE_LIBDIR})
