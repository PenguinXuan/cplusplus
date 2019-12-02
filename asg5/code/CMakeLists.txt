cmake_minimum_required(VERSION 3.15)
project(code)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(code
        debug.cpp
        debug.h
        graphics.cpp
        graphics.h
        interp.cpp
        interp.h
        main.cpp
        rgbcolor.cpp
        rgbcolor.h
        shape.cpp
        shape.h
        util.cpp
        util.h
        util.tcc)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )

target_link_libraries(code ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} )