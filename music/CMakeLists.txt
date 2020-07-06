#project(deepin-music)
set(CMD_NAME deepin-music)
#可执行文件路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/music/dist/bin)

include_directories(${CMAKE_INCLUDE_CURRENT_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Widgets)
find_package(Qt5Svg)
find_package(Qt5Multimedia)
find_package(Qt5Xml)
find_package(Qt5Network)
find_package(Qt5Sql)
find_package(Qt5DBus)
find_package(Qt5LinguistTools)
find_package(Qt5X11Extras)

#文件夹包含
set(PROJECT_INCLUDE ./#暂未使用(会包含空目录)
    ./interface
    ./libdmusic
    music-player
    vendor
    music-player/core
    music-player/core/util
    music-player/presenter
    music-player/speech
    music-player/view
    ./vendor/mpris-qt/src
    ./vendor/dbusextended-qt/src)

include_directories(/usr/include/libdtk-5.2.0/DWidget)
include_directories(/usr/include/libdtk-5.2.0/DCore)
include_directories(/usr/include/vlc)
include_directories(/usr/include/vlc/plugins)

aux_source_directory(./music-player allSource)
aux_source_directory(./music-player/core allSource)
aux_source_directory(./music-player/core/util allSource)
aux_source_directory(./music-player/core/vlc allSource)
aux_source_directory(./music-player/presenter allSource)
aux_source_directory(./music-player/speech allSource)
aux_source_directory(./music-player/view allSource)
aux_source_directory(./music-player/view/helper allSource)
aux_source_directory(./music-player/view/widget allSource)
aux_source_directory(./music-player/view/widget/delegate allSource)
aux_source_directory(./music-player/view/widget/model allSource)

#list(REMOVE_ITEM allSource "mainwindow.cpp" )#排除./XXX.cpp

find_package(PkgConfig REQUIRED)

pkg_check_modules(Dtk REQUIRED IMPORTED_TARGET dtkwidget)
pkg_check_modules(Dtk REQUIRED IMPORTED_TARGET dtkcore)
pkg_check_modules(Dtk REQUIRED IMPORTED_TARGET dtkgui)
pkg_check_modules(AV REQUIRED IMPORTED_TARGET libavutil libavcodec
    libavformat libcue taglib libvlc )

set(AllQRC
    ./music-player/resource/resource.qrc
    ./music-player/resource/theme/theme.qrc
    ./music-player/icons/icons.qrc
    ./music-player/config.qrc
    )

qt5_add_resources(QRC_FILES ${AllQRC})

file(GLOB TS LIST_DIRECTORIES false translations/${CMD_NAME}*.ts)
set_source_files_properties(${TS}
    PROPERTIES OUTPUT_LOCATION ${PROJECT_SOURCE_DIR}/music/music-player/translations)
#set(translate ${PROJECT_SOURCE_DIR}/music/music-player/translations/deepin-music_zh_CN.ts)
if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    qt5_create_translation(Translate ${translate})#${QRC_FILES} ${allSource}  ${INCLUDE_HEADER} ${translate})
endif ()

file(GLOB INCLUDE_HEADER "./interface/*.h"
    "./libdmusic/*.h")


add_executable(${CMD_NAME} ${QRC_FILES} ${allSource}  ${INCLUDE_HEADER} ${Translate})

#添加第三方库的所有文件夹路径到工程中来(注意 *_INCLUDE_DIRS)
target_include_directories(${CMD_NAME} PUBLIC  ${PROJECT_INCLUDE} )

#将工程与这些模块链接起来
qt5_use_modules(${CMD_NAME} ${QtModule})
#X11 Xext Xtst
set(TARGET_LIBS X11 Xext Qt5::Widgets Qt5::Network Qt5::Core Qt5::Gui Qt5::X11Extras
    Qt5::DBus Qt5::Sql Qt5::Svg Qt5::Multimedia Qt5::Xml PkgConfig::AV PkgConfig::Dtk
    dmusic dbusextended-qt5 mpris-qt5 vlccore)


#将第三方库链接进来(注意 *_LIBRARIES)
target_link_libraries(${CMD_NAME} ${TARGET_LIBS} )

install(TARGETS ${CMD_NAME} DESTINATION bin)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/music/music-player/translations
    DESTINATION share/${CMD_NAME}
    FILES_MATCHING PATTERN "*.qm")
install(FILES ${PROJECT_SOURCE_DIR}/music/music-player/data/deepin-music.desktop
    DESTINATION share/applications)

install(FILES ${PROJECT_SOURCE_DIR}/music/music-player/resource/theme/common/image/deepin-music.svg
    DESTINATION share/icons/hicolor/scalable/apps
    RENAME deepin-music.svg)

add_subdirectory(libdmusic)
add_subdirectory(vendor/dbusextended-qt/src)
add_subdirectory(vendor/mpris-qt/src)
