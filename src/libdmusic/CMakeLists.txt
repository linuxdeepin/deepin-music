project(libdmusic)
#工程名称
set(CMD_NAME dmusic)
add_definitions(-DLIBDMUSIC_LIBRARY)

set(QT_MODULES Core Multimedia DBus Sql Core5Compat)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

file(GLOB_RECURSE QRC_SOURCE_FILES ./*.qrc)
set(QRC_FILES config.qrc)

#文件夹包含
include_directories(/usr/include/vlc)
include_directories(/usr/include/vlc/plugins)
include_directories(/runtime/include/vlc)
include_directories(/runtime/include/vlc/plugins)
include_directories(/runtime/include)
include_directories(.)
include_directories(./util)
include_directories(./core)
include_directories(./player)
include_directories(./player/vlc)

aux_source_directory(. ALL_SOURCES)
aux_source_directory(./util ALL_SOURCES)
aux_source_directory(./core ALL_SOURCES)
aux_source_directory(./player ALL_SOURCES)
aux_source_directory(./player/vlc ALL_SOURCES)

#使用第三方库需要用到的一个包
find_package(PkgConfig REQUIRED)

find_package(Qt6 REQUIRED COMPONENTS ${QT_MODULES})
find_package(ICU COMPONENTS i18n)
qt_add_resources(QRC_FILES ${QRC_SOURCE_FILES})

add_library(${CMD_NAME} SHARED ${ALL_SOURCES} ${QRC_FILES})
set_target_properties(dmusic PROPERTIES VERSION 1.0.0 SOVERSION 1.0)

pkg_check_modules(MPRIS REQUIRED IMPORTED_TARGET mpris-qt6)
pkg_check_modules(TAGLIB REQUIRED IMPORTED_TARGET taglib)
pkg_check_modules(DTK REQUIRED IMPORTED_TARGET dtk6core)
# pkg_check_modules(udisks2 REQUIRED IMPORTED_TARGET udisks2-qt5)

set(TARGET_LIBS PkgConfig::TAGLIB PkgConfig::DTK)
target_link_libraries(${CMD_NAME} ${TARGET_LIBS} PkgConfig::MPRIS PkgConfig::TAGLIB PkgConfig::DTK ICU::i18n)
target_link_libraries(${CMD_NAME} Qt6::Core Qt6::Multimedia Qt6::DBus Qt6::Sql Qt6::Core5Compat)

# qt5_use_modules(${CMD_NAME} )

include(GNUInstallDirs)
install(TARGETS ${CMD_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
