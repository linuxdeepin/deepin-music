#定义需要的cmake版本
#cmake_minimum_required(VERSION 3.10)

if(ISTEST MATCHES "1")
    set(CMD_NAME deepin-music-test)
else()
    set(CMD_NAME deepin-music)
endif()

#可执行文件路径
#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/music/dist/bin)

#cmake属性
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

#文件夹包含
set(PROJECT_INCLUDE ./
    ./../vendor
    ./../vendor/mpris-qt/src
    ./../vendor/dbusextended-qt/src)

#include_directories(/usr/include/libdtk-5.2.0/DWidget)
#include_directories(/usr/include/libdtk-5.2.0/DCore)
include_directories(/usr/include/vlc)
include_directories(/usr/include/vlc/plugins)
include_directories(./../interface)
include_directories(./../libdmusic)
include_directories(./core)
include_directories(./core/util)
include_directories(./presenter)
include_directories(./speech)
include_directories(./view)

if(ISTEST MATCHES "1")
    message("tests start！")
    file(GLOB mainSource
        ./musicapp.cpp
        ../../tests/*.cpp)
else()
    aux_source_directory(./ allSource)
endif()

aux_source_directory(./core allSource)
aux_source_directory(./core/util allSource)
aux_source_directory(./core/vlc allSource)
aux_source_directory(./presenter allSource)
aux_source_directory(./speech allSource)
aux_source_directory(./view allSource)
aux_source_directory(./view/helper allSource)
aux_source_directory(./view/widget allSource)
aux_source_directory(./view/widget/delegate allSource)
aux_source_directory(./view/widget/model allSource)

find_package(PkgConfig REQUIRED)

pkg_check_modules(Dtk REQUIRED IMPORTED_TARGET dtkwidget)
pkg_check_modules(Dtk REQUIRED IMPORTED_TARGET dtkcore)
pkg_check_modules(Dtk REQUIRED IMPORTED_TARGET dtkgui)
pkg_check_modules(qgsettings REQUIRED IMPORTED_TARGET gsettings-qt)

#pkg_check_modules(AV REQUIRED IMPORTED_TARGET libavutil libavcodec
#    libavformat libcue taglib)

pkg_check_modules(AV REQUIRED IMPORTED_TARGET libcue taglib)

set(AllQRC
    ./resource/resource.qrc
    ./resource/theme/theme.qrc
    ./icons/icons.qrc
    ./config.qrc
    )

qt5_add_resources(QRC_FILES ${AllQRC})

# 设置不删除生成的文件夹内容文件（设置此项是为了翻译后的文件不被删除）
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

#set(TS
#    translations/${CMD_NAME}.ts
#    translations/${CMD_NAME}_zh_CN.ts
#    translations/${CMD_NAME}_en_US.ts
#    )

#set_source_files_properties(${qmfiles} PROPERTIES OUTPUT_LOCATION "translations")

file(GLOB TS LIST_DIRECTORIES false ./translations/${CMD_NAME}*.ts)
set_source_files_properties(${TS}
    PROPERTIES OUTPUT_LOCATION ${PROJECT_SOURCE_DIR}/music/music-player/translations)
set(translate ${PROJECT_SOURCE_DIR}/music/music-player/translations)

if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    qt5_create_translation(Translate1 ${allSource} ${TS} OPTIONS -source-language en_US -no-obsolete)
endif ()
qt5_add_translation(${translate})

file(GLOB INCLUDE_HEADER "./../interface/*.h")

add_executable(${CMD_NAME} ${QRC_FILES} ${allSource} ${INCLUDE_HEADER} ${Translate1} ${mainSource})

#添加第三方库的所有文件夹路径到工程中来(注意 *_INCLUDE_DIRS)
target_include_directories(${CMD_NAME} PUBLIC  ${PROJECT_INCLUDE} )

#将工程与这些模块链接起来
qt5_use_modules(${CMD_NAME} ${QtModule})
#X11 Xext Xtst  Qt5::Concurrent
set(TARGET_LIBS X11 Xext Qt5::Widgets Qt5::Core Qt5::Gui Qt5::X11Extras
    Qt5::DBus  Qt5::Sql Qt5::Svg Qt5::Multimedia Qt5::Xml PkgConfig::AV PkgConfig::Dtk PkgConfig::qgsettings icui18n
    mpris-qt5 dbusextended-qt5 dmusic)

LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/music/libdmusic/
        ${CMAKE_BINARY_DIR}/music/vendor/dbusextended-qt/src/
        ${CMAKE_BINARY_DIR}/music/vendor/mpris-qt/src/)


#将第三方库链接进来(注意 *_LIBRARIES)
target_link_libraries(${CMD_NAME} ${TARGET_LIBS} )

if(ISTEST MATCHES "1")
    target_link_libraries(${CMD_NAME} gmock gmock_main gtest gtest_main)
endif()

install(TARGETS ${CMD_NAME} DESTINATION bin)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/music/music-player/translations
    DESTINATION share/${CMD_NAME}
    FILES_MATCHING PATTERN "*.qm")
#install(FILES ${Translate1} DESTINATION share/${CMD_NAME})
install(FILES ${PROJECT_SOURCE_DIR}/music/music-player/data/deepin-music.desktop
    DESTINATION share/applications)

install(FILES ${PROJECT_SOURCE_DIR}/music/music-player/resource/theme/common/image/deepin-music.svg
    DESTINATION share/icons/hicolor/scalable/apps)
