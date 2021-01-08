#工程名
set(CMD_NAME weather-plugin)
set(CMAKE_INSTALL_VOICE_LIBDIR /usr/lib/deepin-aiassistant/serivce-plugins)
cmake_minimum_required(VERSION 3.10)

#文件夹包含
include_directories(.)
include_directories(./interface)

aux_source_directory(. allSourcelib)

add_library(${CMD_NAME} SHARED ${allSourcelib})

#使用第三方库需要用到的一个包
find_package(PkgConfig REQUIRED)

#检查第三方库(这里检查了名字为dtkwidget的库和名字为dtkgui的库)，然后取名3rd_lib
pkg_check_modules(3rd_lib REQUIRED  dframeworkdbus dtkcore)
#将第三方库链接进来(注意 *_LIBRARIES)
target_link_libraries(${CMD_NAME} ${3rd_lib_LIBRARIES})
#target_link_libraries(${CMD_NAME} sys_semantic_inc)


qt5_use_modules(${CMD_NAME} ${QtModule})

#install(TARGETS ${CMD_NAME} DESTINATION ${CMAKE_INSTALL_VOICE_LIBDIR})
