cmake_minimum_required(VERSION 3.19)
project(F3D)

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)


set(CMAKE_CXX_STANDARD 17) # C++17

# scan for all source files in src/
file(GLOB_RECURSE SOURCES
    src/*.cpp
    src/*.c
    src/*.h)

# Ensure new material property implementation is included.
list(APPEND SOURCES
    src/resource/materials/material_properties.cpp)


add_executable(F3D ${SOURCES})

# add_definitions(-DDEBUG)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDOUBLE_PRECISION")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W")

target_include_directories(F3D PUBLIC "${CMAKE_SOURCE_DIR}/include/")

target_link_libraries(F3D glfw OpenGL::GL)

# debugging symbols
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# set(CMAKE_EXE_LINKER_FLAGS " -static")
