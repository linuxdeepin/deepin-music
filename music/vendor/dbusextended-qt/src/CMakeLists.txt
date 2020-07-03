cmake_minimum_required(VERSION 3.10)

project(dbusextended-qt5)

#文件路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../../../dist/lib)
#文件夹包含
include_directories(./)

aux_source_directory(./ allSourcelib2)

add_library(dbusextended-qt5 SHARED ${allSourcelib2})

#使用第三方库需要用到的一个包
find_package(PkgConfig REQUIRED)

#检查第三方库(这里检查了名字为dtkwidget的库和名字为dtkgui的库)，然后取名3rd_lib
pkg_check_modules(3rd_lib REQUIRED
        dtkwidget dtkgui
        )
#添加第三方库的所有文件夹路径到工程中来(注意 *_INCLUDE_DIRS)
target_include_directories(dbusextended-qt5 PUBLIC ${3rd_lib_INCLUDE_DIRS}
    )

#将第三方库链接进来(注意 *_LIBRARIES)
target_link_libraries(dbusextended-qt5 ${3rd_lib_LIBRARIES})

#将工程与这些模块链接起来
qt5_use_modules(dbusextended-qt5 ${QtModule})
install(TARGETS dbusextended-qt5 DESTINATION /lib/)
