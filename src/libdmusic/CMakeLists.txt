project(libdmusic)
#工程名称
set(CMD_NAME dmusic)
add_definitions(-DLIBDMUSIC_LIBRARY -DVLCQT_CORE_LIBRARY)

set(QT_MODULES Core Multimedia DBus Sql Core5Compat)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
# Disable AUTORCC since qt_add_resources is used explicitly
set(CMAKE_AUTORCC OFF)

file(GLOB_RECURSE QRC_SOURCE_FILES ./*.qrc)
set(QRC_FILES config.qrc)

#文件夹包含
find_path(VLC_INCLUDE_DIR vlc/vlc.h)
if(VLC_INCLUDE_DIR)
  include_directories(${VLC_INCLUDE_DIR})
  if(EXISTS "${VLC_INCLUDE_DIR}/vlc/plugins")
    include_directories(${VLC_INCLUDE_DIR}/vlc/plugins)
  elseif(EXISTS "${VLC_INCLUDE_DIR}/plugins")
    include_directories(${VLC_INCLUDE_DIR}/plugins)
  endif()
  # Also add vlc/ dir for libvlc_media.h etc.
  if(EXISTS "${VLC_INCLUDE_DIR}/vlc")
    include_directories(${VLC_INCLUDE_DIR}/vlc)
  endif()
endif()
# Windows: force-include poll() shim (select-based) so VLC headers compile
if(WIN32)
  add_compile_options(-include poll.h)
endif()
if(EXISTS "/runtime/include/vlc")
  include_directories(/runtime/include/vlc)
  include_directories(/runtime/include/vlc/plugins)
  include_directories(/runtime/include)
endif()
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

# Windows: add compat include directory after target creation
if(WIN32)
  target_include_directories(${CMD_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src/compat")
endif()

# On MinGW, export all symbols since VLC wrapper classes lack export macros
if(WIN32 AND NOT MSVC)
    target_link_options(${CMD_NAME} PRIVATE -Wl,--export-all-symbols)
endif()
set_target_properties(dmusic PROPERTIES VERSION 1.0.0 SOVERSION 1.0)

if(LINUX)
  pkg_check_modules(MPRIS REQUIRED IMPORTED_TARGET mpris-qt6)
endif()
pkg_check_modules(TAGLIB REQUIRED IMPORTED_TARGET taglib)
pkg_check_modules(DTK REQUIRED IMPORTED_TARGET dtk6core)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET libavcodec libavformat)
# pkg_check_modules(udisks2 REQUIRED IMPORTED_TARGET udisks2-qt5)

set(TARGET_LIBS PkgConfig::TAGLIB PkgConfig::DTK)
target_link_libraries(${CMD_NAME} ${TARGET_LIBS} PkgConfig::TAGLIB PkgConfig::DTK ICU::i18n PkgConfig::FFMPEG)
if(LINUX)
  target_link_libraries(${CMD_NAME} PkgConfig::MPRIS)
endif()
target_link_libraries(${CMD_NAME} Qt6::Core Qt6::Multimedia Qt6::DBus Qt6::Sql Qt6::Core5Compat)

# qt5_use_modules(${CMD_NAME} )

# Windows: always link ws2_32 for Winsock APIs (poll shim, networking)
if(WIN32)
    target_link_libraries(${CMD_NAME} ws2_32)
endif()

include(GNUInstallDirs)
install(TARGETS ${CMD_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
