cmake_minimum_required(VERSION 3.16)
project(axiorem C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Use pkg-config to find Raylib
find_package(PkgConfig REQUIRED)
pkg_check_modules(RAYLIB REQUIRED IMPORTED_TARGET raylib)

# Find cJSON library (installed via brew)
find_library(CJSON_LIBRARY
    NAMES cjson
    PATHS /usr/local/lib /opt/homebrew/lib
)

find_path(CJSON_INCLUDE_DIR
    NAMES cjson/cJSON.h
    PATHS /usr/local/include /opt/homebrew/include
)

if(NOT CJSON_LIBRARY OR NOT CJSON_INCLUDE_DIR)
    message(FATAL_ERROR "cJSON library not found. Make sure it's installed via 'brew install cjson'")
endif()

message(STATUS "Found cJSON: ${CJSON_LIBRARY}")
message(STATUS "cJSON include dir: ${CJSON_INCLUDE_DIR}")

# Add all source files from the refactored modular structure
add_executable(axiorem 
    src/main.c
    src/client/sim_loader.c
    src/render/game_window.c
    src/render/camera.c
    src/render/renderer.c
    src/render/ui.c
    src/utils/math_utils.c
)

# Include directories for the modular structure
target_include_directories(axiorem PRIVATE 
    src
    src/client
    src/render
    src/utils
    ${RAYLIB_INCLUDE_DIRS}
    ${CJSON_INCLUDE_DIR}
)

# Link via pkg-config and add cJSON
target_link_libraries(axiorem PRIVATE 
    PkgConfig::RAYLIB
    ${CJSON_LIBRARY}
)

# Add math library for math functions (sqrtf, etc.)
target_link_libraries(axiorem PRIVATE m)

# Compiler options for better code quality
target_compile_options(axiorem PRIVATE 
    -Wall
    -Wextra
    -Wpedantic
    -Werror=return-type
)

# Optional: Create bundle only if explicitly requested
option(BUILD_AS_BUNDLE "Build as macOS application bundle" OFF)
if(APPLE AND BUILD_AS_BUNDLE)
    set_target_properties(axiorem PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.axiom.axiorem"
        MACOSX_BUNDLE_BUNDLE_NAME "Axiorem"
    )
endif()

# Optional: Debug configuration
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(axiorem PRIVATE DEBUG)
    target_compile_options(axiorem PRIVATE -g -O0)
else()
    target_compile_options(axiorem PRIVATE -O2)
endif()
