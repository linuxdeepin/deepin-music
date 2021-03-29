#定义需要的cmake版本
#cmake_minimum_required(VERSION 3.10)

if(ISTEST MATCHES "1")
    set(CMD_NAME deepin-music-test)
    add_definitions(-DENABLE_AUTO_UNIT_TEST)
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

ADD_DEFINITIONS(-DENABLE_ACCESSIBILITY=0)

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
include_directories(./tablet)
include_directories(./view)
include_directories(./mainFrame)
include_directories(./widget)
include_directories(./widget/lrc)
include_directories(./listView)
include_directories(./listView/musicBaseAndSongList)
include_directories(./listView/musicInfoList)
include_directories(./listView/musicSearchList)
include_directories(./listView/albumList)
include_directories(./listView/singerList)
include_directories(./listView/lyricLineList)

include_directories(/usr/include/udisks2-qt5)

if(ISTEST MATCHES "1")
    message("tests start！")
    file(GLOB mainSource
        ./musicapp.cpp
        ../../tests/*.cpp
        ../../tests/speech/*.cpp
        ../../tests/presenter/*.cpp)
else()
    aux_source_directory(./ allSource)
endif()

aux_source_directory(./core allSource)
aux_source_directory(./core/util allSource)
aux_source_directory(./core/plugin allSource)
aux_source_directory(./core/vlc allSource)
aux_source_directory(./presenter allSource)
aux_source_directory(./speech allSource)
aux_source_directory(./tablet allSource)
aux_source_directory(./mainFrame allSource)
aux_source_directory(./listView allSource)
aux_source_directory(./widget allSource)
aux_source_directory(./widget/helper allSource)
aux_source_directory(./widget/lrc allSource)
aux_source_directory(./view allSource)
aux_source_directory(./listView allSource)
aux_source_directory(./listView/musicBaseAndSongList allSource)
aux_source_directory(./listView/musicInfoList allSource)
aux_source_directory(./listView/albumList allSource)
aux_source_directory(./listView/singerList allSource)
aux_source_directory(./view/widget allSource)
aux_source_directory(./view/widget/delegate allSource)
aux_source_directory(./view/widget/model allSource)
aux_source_directory(./listView/musicSearchList allSource)
aux_source_directory(./listView/lyricLineList allSource)

find_package(PkgConfig REQUIRED)

pkg_check_modules(Dtk REQUIRED IMPORTED_TARGET dtkwidget)
pkg_check_modules(Dtk REQUIRED IMPORTED_TARGET dtkcore)
pkg_check_modules(Dtk REQUIRED IMPORTED_TARGET dtkgui)
pkg_check_modules(MPRIS REQUIRED IMPORTED_TARGET mpris-qt5)
pkg_check_modules(DBusextended REQUIRED IMPORTED_TARGET dbusextended-qt5)
pkg_check_modules(VLC REQUIRED IMPORTED_TARGET vlc-plugin)
pkg_check_modules(qgsettings REQUIRED IMPORTED_TARGET gsettings-qt)

#pkg_check_modules(AV REQUIRED IMPORTED_TARGET libavutil libavcodec
#    libavformat libcue taglib)

pkg_check_modules(AV REQUIRED IMPORTED_TARGET libcue taglib)

pkg_check_modules(udisks2 REQUIRED IMPORTED_TARGET udisks2-qt5)


set(AllQRC
#    ./resource/resource.qrc
#    ./resource/theme/theme.qrc
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

file(GLOB TS LIST_DIRECTORIES false ../../translations/${CMD_NAME}*.ts)
set_source_files_properties(${TS}
    PROPERTIES OUTPUT_LOCATION ${PROJECT_SOURCE_DIR}/translations)
set(translate ${PROJECT_SOURCE_DIR}/translations)

if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    qt5_create_translation(Translate1 ${allSource} ${TS} OPTIONS -source-language en_US -no-obsolete)
endif ()
qt5_add_translation(${translate})

#file(GLOB INCLUDE_HEADER "./../interface/*.h")

add_executable(${CMD_NAME} ${QRC_FILES} ${allSource} ${INCLUDE_HEADER} ${Translate1} ${mainSource})

#添加第三方库的所有文件夹路径到工程中来(注意 *_INCLUDE_DIRS)
target_include_directories(${CMD_NAME} PUBLIC  ${PROJECT_INCLUDE} )

#将工程与这些模块链接起来
qt5_use_modules(${CMD_NAME} ${QtModule})

set(TARGET_LIBS Qt5::Widgets Qt5::Core Qt5::Gui Qt5::DBus Qt5::Sql Qt5::Svg Qt5::Xml
     PkgConfig::AV PkgConfig::Dtk PkgConfig::qgsettings PkgConfig::udisks2 icui18n
    PkgConfig::MPRIS PkgConfig::DBusextended pthread dmusic )
LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/src/libdmusic/)


#将第三方库链接进来(注意 *_LIBRARIES)
target_link_libraries(${CMD_NAME} ${TARGET_LIBS} )

if(ISTEST MATCHES "1")
    set(GTEST_LIBS gmock  gtest)
    target_link_libraries(${CMD_NAME} ${GTEST_LIBS})
endif()

# ------------------------------- install script -------------------------------
install(TARGETS ${CMD_NAME} DESTINATION bin)
# install Translation files:
install(DIRECTORY ${PROJECT_SOURCE_DIR}/translations
    DESTINATION share/${CMD_NAME}
    FILES_MATCHING PATTERN "*.qm")
#install(FILES ${Translate1} DESTINATION share/${CMD_NAME})
# install Desktop icon
install(FILES ${PROJECT_SOURCE_DIR}/src/music-player/data/deepin-music.desktop
    DESTINATION share/applications)
# install Manual App icon
install(FILES ${PROJECT_SOURCE_DIR}/src/music-player/icons/icons/deepin-music.svg
    DESTINATION share/icons/hicolor/scalable/apps)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/assets/deepin-music DESTINATION /usr/share/deepin-manual/manual-assets/application/)
