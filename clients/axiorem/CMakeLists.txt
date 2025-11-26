# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(axiorem C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Use pkg-config to find Raylib
find_package(PkgConfig REQUIRED)
pkg_check_modules(RAYLIB REQUIRED IMPORTED_TARGET raylib)

add_executable(axiorem src/main.c)

# Link via pkg-config
target_link_libraries(axiorem PRIVATE PkgConfig::RAYLIB)
target_include_directories(axiorem PRIVATE ${RAYLIB_INCLUDE_DIRS})
