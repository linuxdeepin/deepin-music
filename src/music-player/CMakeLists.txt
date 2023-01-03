set(APP_BIN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/bin/)
set(BIN_NAME ${CMAKE_PROJECT_NAME})
list(APPEND RESOURCES ${CMAKE_PROJECT_NAME}.qrc)
list(APPEND RESOURCES dci.qrc)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Find the Qt5Quick library
find_package(DtkDeclarative REQUIRED)
find_package(Qt5 COMPONENTS Quick QuickCompiler LinguistTools REQUIRED)

set(QT_MODULES Core Widgets)
find_package(Qt5 REQUIRED ${QT_MODULES})

find_package(PkgConfig REQUIRED)
pkg_check_modules(DTK REQUIRED IMPORTED_TARGET dtkwidget)
pkg_check_modules(DTK REQUIRED IMPORTED_TARGET dtkcore)
pkg_check_modules(DTK REQUIRED IMPORTED_TARGET dtkgui)

qtquick_compiler_add_resources(RCC_SOURCES ${RESOURCES})

# 保证 src 目录下头文件全局可见
include_directories(src)
include_directories(../libdmusic)
include_directories(../util)

aux_source_directory(./ ALLSOURCE)
aux_source_directory(./effect ALLSOURCE)
aux_source_directory(./util ALLSOURCE)

LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/src/libdmusic)

file(GLOB TS LIST_DIRECTORIES false ${PROJECT_SOURCE_DIR}/translations/${BIN_NAME}*.ts)
set_source_files_properties(${TS}
    PROPERTIES OUTPUT_LOCATION ${PROJECT_SOURCE_DIR}/translations)
set(translate ${PROJECT_SOURCE_DIR}/translations)

if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    qt5_add_translation(QM_FILES ${TS})
else()
    qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS} OPTIONS -source-language en_US -no-obsolete)
endif ()

# Add EXE
add_executable(${BIN_NAME} ${RESOURCES} ${ALLSOURCE} ${QM_FILES})

qt5_use_modules(${BIN_NAME} ${QT_MODULES})

set(TARGET_LIBS Qt5::Quick PkgConfig::DTK dmusic ${DtkDeclarative_LIBRARIES})

target_link_libraries(${BIN_NAME} ${TARGET_LIBS})

set_target_properties(${BIN_NAME} PROPERTIES INSTALL_RPATH ${DTK_QML_APP_PLUGIN_PATH})

# install dconfig meta files
dconfig_meta_files(APPID deepin-music FILES data/org.deepin.music.json)

# Add install rule
install(TARGETS ${BIN_NAME} DESTINATION ${APP_BIN_INSTALL_DIR})
# install Translation files
install(DIRECTORY ${PROJECT_SOURCE_DIR}/translations DESTINATION share/${CMAKE_PROJECT_NAME} FILES_MATCHING PATTERN "*.qm")
# install Desktop icon
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/${CMAKE_PROJECT_NAME}.desktop DESTINATION share/applications)
# install Manual App icon
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/${CMAKE_PROJECT_NAME}.svg DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/assets/deepin-music DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin-manual/manual-assets/application/)
