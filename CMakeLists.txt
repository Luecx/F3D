cmake_minimum_required(VERSION 3.19)
project(F3D)

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)


set(CMAKE_CXX_STANDARD 17)

# scan for all source files in src/
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h" "src/*.c")

add_executable(F3D ${SOURCES}
        src/math/mat.h
        src/math/mat_transform.impl
        src/math/mat_project.impl
        src/math/transformation.cpp
        src/math/transformation.h
)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDOUBLE_PRECISION")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W")

target_link_libraries(F3D glfw OpenGL::GL)

# debugging symbols
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# set(CMAKE_EXE_LINKER_FLAGS " -static")
