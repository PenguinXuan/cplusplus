cmake_minimum_required(VERSION 3.15)
project(asgn2)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_custom_target(asgn2 COMMAND make WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
