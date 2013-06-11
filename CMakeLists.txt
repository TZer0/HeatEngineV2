CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(HeatEngineV2)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/;${CMAKE_SOURCE_DIR}/cmake/Utils/")

find_package(OGRE REQUIRED)
find_package(OIS REQUIRED)
find_package(OPENAL REQUIRED)

add_definitions(-std=gnu++0x -Wall)
include_directories(${OGRE_INCLUDE_DIRS})
include_directories(${OIS_INCLUDE_DIRS})
include_directories(${OPENAL_INCLUDE_DIRS})

add_subdirectory(src/)
