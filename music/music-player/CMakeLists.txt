#定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

#工程名
project(deepin-music)

#cmake属性
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

#可执行文件路径
set(EXECUTABLE_OUTPUT_PATH /home/tx-zy/work/masterCMake/src/dist/bin)

set(EXE_USED_MYCODE_PATH
    ./../libdmusic
    ./../interface
    ./../vendor/mpris-qt/src
    /usr/include/libdtk-5.2.0/DWidget
    /usr/include/libdtk-5.2.0/DCore)

#文件夹包含
include_directories(./)
include_directories(./interface)
include_directories(core)
include_directories(core/util)
include_directories(presenter)
include_directories(speech)
include_directories(view)

aux_source_directory(. allSource)
aux_source_directory(./../interfae allSource)
aux_source_directory(core allSource)
aux_source_directory(core/util allSource)
aux_source_directory(presenter allSource)
aux_source_directory(speech allSource)
aux_source_directory(view allSource)

set(AllQRC
    ./resource/resource.qrc
    ./resource/theme/theme.qrc
    ./icons/icons.qrc
    ./config.qrc
    )

qt5_add_resources(QRC_FILES ${AllQRC})

add_executable(${PROJECT_NAME} ${QRC_FILES} ${allSource} ${EXE_USED_MYCODE_PATH})

#使用第三方库需要用到的一个包
find_package(PkgConfig REQUIRED)

#检查第三方库(这里检查了名字为dtkwidget的库和名字为dtkgui的库)，然后取名3rd_lib
pkg_check_modules(3rd_lib REQUIRED
        dtkwidget dtkgui
        )
#添加第三方库的所有文件夹路径到工程中来(注意 *_INCLUDE_DIRS)
target_include_directories(${PROJECT_NAME} PUBLIC ${3rd_lib_INCLUDE_DIRS}
    ${EXE_USED_MYCODE_PATH})

#将第三方库链接进来(注意 *_LIBRARIES)
target_link_libraries(${PROJECT_NAME} ${3rd_lib_LIBRARIES} dmusic dbusextended-qt5 mpris-qt5)

#将工程与这些模块链接起来
qt5_use_modules(${PROJECT_NAME} ${QtModule})

add_subdirectory(libdmusic)
add_subdirectory(vendor/dbusextended-qt/src)
add_subdirectory(vendor/mpris-qt/src)

