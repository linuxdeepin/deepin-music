cmake_minimum_required(VERSION 3.10)
#工程名
project(mpris-qt5)

#文件路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/dist/lib)#${PROJECT_BINARY_DIR}./../../dist/lib)
set(MY_DBUS_PATH ./../../dbusextended-qt/src)
#文件夹包含
include_directories(./)

aux_source_directory(./ allSourcelib1)

add_library(mpris-qt5 SHARED ${allSourcelib1})

#使用第三方库需要用到的一个包
find_package(PkgConfig REQUIRED)

#检查第三方库(这里检查了名字为dtkwidget的库和名字为dtkgui的库)，然后取名3rd_lib
pkg_check_modules(3rd_lib REQUIRED
        dtkwidget dtkgui #libdbusextended-qt5 libdmusic libmpris-qt5
        )
#添加第三方库的所有文件夹路径到工程中来(注意 *_INCLUDE_DIRS)
target_include_directories(mpris-qt5 PUBLIC ${3rd_lib_INCLUDE_DIRS}
    ${MY_DBUS_PATH})

#将第三方库链接进来(注意 *_LIBRARIES)
target_link_libraries(mpris-qt5 ${3rd_lib_LIBRARIES})

#将工程与这些模块链接起来
qt5_use_modules(mpris-qt5 ${QtModule})
