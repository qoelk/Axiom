# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(axiorem C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Use pkg-config to find Raylib
find_package(PkgConfig REQUIRED)
pkg_check_modules(RAYLIB REQUIRED IMPORTED_TARGET raylib)

# Add all source files
add_executable(axiorem 
    src/main.c
    src/client/sim_loader.c
    src/render/game_window.c
)

# Include directories
target_include_directories(axiorem PRIVATE 
    src
    src/client
    src/render
    src/utils
    ${RAYLIB_INCLUDE_DIRS}
)

# Link via pkg-config
target_link_libraries(axiorem PRIVATE PkgConfig::RAYLIB)

# Optional: Create bundle only if explicitly requested
option(BUILD_AS_BUNDLE "Build as macOS application bundle" OFF)
if(APPLE AND BUILD_AS_BUNDLE)
    set_target_properties(axiorem PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.axiom.axiorem"
        MACOSX_BUNDLE_BUNDLE_NAME "Axiorem"
    )
endif()
