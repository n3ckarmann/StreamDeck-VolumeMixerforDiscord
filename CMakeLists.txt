cmake_minimum_required(VERSION 3.20)

# Builds with Qt 6.8 LTS: on Windows with MSVC 2022 or MinGW 13 (x64), on macOS with Xcode clang (universal binary)

if (APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum macOS version")
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build a universal binary")
endif ()

project(streamdeck-volumemixer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_IN_STREAMDECK_PLUGINS_DIR "Whether to build to the 'bin' directory or to the stream deck plugins directory." OFF)

# ===========================================
# Target setup
# ===========================================
set(target streamdeck-volumemixer)

# Qt setup
find_package(Qt6 COMPONENTS Core WebSockets Network Gui REQUIRED CONFIG)
set(CMAKE_AUTOMOC ON)

if (BUILD_IN_STREAMDECK_PLUGINS_DIR AND APPLE)
    set(target_directory "$ENV{HOME}/Library/Application Support/com.elgato.StreamDeck/Plugins/com.n3ckarmann.volumemixer.sdPlugin")
elseif (BUILD_IN_STREAMDECK_PLUGINS_DIR)
    file(TO_CMAKE_PATH "$ENV{APPDATA}" appdata_path)
    set(target_directory "${appdata_path}/Elgato/StreamDeck/Plugins/com.n3ckarmann.volumemixer.sdPlugin")
else ()
    set(target_directory "${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/com.n3ckarmann.volumemixer.sdPlugin")
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${target_directory}/bin")
qt_add_executable(${target})
target_link_libraries(${target} PRIVATE Qt6::WebSockets Qt6::Network Qt6::Gui)
target_compile_definitions(${target} PRIVATE QT_DISABLE_DEPRECATED_BEFORE=0x060200)

if (APPLE)
    # Stream Deck starts <plugin>/bin/streamdeck-volumemixer.app/Contents/MacOS/streamdeck-volumemixer (see CodePathMac)
    set_target_properties(${target} PROPERTIES
            MACOSX_BUNDLE ON
            MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/etc/macos/Info.plist.in"
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.n3ckarmann.volumemixer"
            MACOSX_BUNDLE_BUNDLE_NAME "Volume Mixer for Discord")
endif ()

# ===========================================
# Sources
# ===========================================
file(GLOB_RECURSE source_files CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp")
target_sources(${target} PRIVATE ${source_files})
target_include_directories(${target} PRIVATE "${PROJECT_SOURCE_DIR}/src")

# Plugin texts in all languages, compiled into the executable
qt_add_resources(${target} "translations" PREFIX "/" BASE "${PROJECT_SOURCE_DIR}/src" FILES "${PROJECT_SOURCE_DIR}/src/translations.json")

# ===========================================
# QtStreamDeck
# ===========================================
file(GLOB_RECURSE streamdeckplugin_files CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/deps/qtstreamdeck2/*.cpp")
target_sources(${target} PRIVATE ${streamdeckplugin_files})
target_include_directories(${target} PRIVATE "${PROJECT_SOURCE_DIR}/deps/qtstreamdeck2")

# ===========================================
# QtDiscord
# ===========================================
file(GLOB_RECURSE discordipc_files CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/deps/qtdiscordipc/*.cpp")
target_sources(${target} PRIVATE ${discordipc_files})
target_include_directories(${target} PRIVATE "${PROJECT_SOURCE_DIR}/deps/qtdiscordipc")

# ===========================================
# Deployment
# ===========================================

# Copy files from dist
install(DIRECTORY "${PROJECT_SOURCE_DIR}/dist/" DESTINATION ${target_directory})
install(DIRECTORY "${PROJECT_SOURCE_DIR}/deps/qtstreamdeck2/dist/" DESTINATION ${target_directory})

# Run windeployqt / macdeployqt
get_target_property(QMAKE_FILEPATH Qt6::qmake IMPORTED_LOCATION)

install(CODE "set(target_file \"$<TARGET_FILE:${target}>\")")
if (APPLE)
    install(CODE "set(target_bundle \"$<TARGET_BUNDLE_DIR:${target}>\")")
endif ()
install(CODE "set(QMAKE_FILEPATH \"${QMAKE_FILEPATH}\")")
install(CODE "set(PROJECT_SOURCE_DIR \"${PROJECT_BASE_DIR}\")")
install(CODE "set(CMAKE_BINARY_DIR \"${CMAKE_BASE_DIR}\")")
install(SCRIPT "${PROJECT_SOURCE_DIR}/deploy.cmake")