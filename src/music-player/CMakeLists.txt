set(APP_BIN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/bin/)
set(BIN_NAME ${CMAKE_PROJECT_NAME})
list(APPEND RESOURCES ${CMAKE_PROJECT_NAME}.qrc)
list(APPEND RESOURCES dci.qrc)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Find the Qt6Quick library
find_package(Dtk6Declarative REQUIRED)
find_package(Qt6 COMPONENTS Core Widgets Quick LinguistTools REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(DTK REQUIRED IMPORTED_TARGET dtk6widget)
pkg_check_modules(DTK REQUIRED IMPORTED_TARGET dtk6core)
pkg_check_modules(DTK REQUIRED IMPORTED_TARGET dtk6gui)

qt6_add_resources(RCC_SOURCES ${RESOURCES})

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
    qt6_add_translation(QM_FILES ${TS})
else()
    qt6_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS} OPTIONS -source-language en_US -no-obsolete)
endif ()

# Add EXE
add_executable(${BIN_NAME} ${RCC_SOURCES} ${ALLSOURCE} ${QM_FILES})

# Use target_link_libraries instead of qt6_use_modules
target_link_libraries(${BIN_NAME} Qt6::Core Qt6::Widgets)

set(TARGET_LIBS Qt6::Quick PkgConfig::DTK dmusic ${Dtk6Declarative_LIBRARIES})

target_link_libraries(${BIN_NAME} ${TARGET_LIBS})

# install dconfig meta files
dtk_add_config_meta_files(APPID deepin-music FILES data/org.deepin.music.json)

# Add install rule
install(TARGETS ${BIN_NAME} DESTINATION ${APP_BIN_INSTALL_DIR})
# install Translation files
install(DIRECTORY ${PROJECT_SOURCE_DIR}/translations DESTINATION share/${CMAKE_PROJECT_NAME} FILES_MATCHING PATTERN "*.qm")
# install Desktop icon
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/${CMAKE_PROJECT_NAME}.desktop DESTINATION share/applications)
# install Manual App icon
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/${CMAKE_PROJECT_NAME}.svg DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/assets/deepin-music DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin-manual/manual-assets/application/)
