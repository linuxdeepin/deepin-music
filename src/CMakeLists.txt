#project(deepin-music)
#set(CMD_NAME deepin-music)

include_directories(${CMAKE_INCLUDE_CURRENT_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Widgets)
find_package(Qt5Svg)
#find_package(Qt5Multimedia)
find_package(Qt5Xml)
find_package(Qt5Sql)
find_package(Qt5DBus)
find_package(Qt5LinguistTools)
#find_package(Qt5X11Extras)

include(GNUInstallDirs)

add_subdirectory(libdmusic)
add_subdirectory(music-player)
add_subdirectory(libmusic-plugin)
