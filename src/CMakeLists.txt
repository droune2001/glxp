cmake_minimum_required (VERSION 3.12)

set( GLXP_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake" )
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${GLXP_MODULE_PATH})
set( CMAKE_PREFIX_PATH ${GLXP_MODULE_PATH})

# default compilation flags
set( CMAKE_USER_MAKE_RULES_OVERRIDE_CXX "${GLXP_MODULE_PATH}/cxx_flag_overrides.cmake" )

project(glxp)

set( CMAKE_DEBUG_POSTFIX d )

set_property( GLOBAL PROPERTY USE_FOLDERS ON )

# bin/lib dirs in each build dir.
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" ) # .exe
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" ) # .dll ou .so
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" ) # .lib windows, .a linux

set( ASSETS_DIR "${CMAKE_SOURCE_DIR}/../data")

find_package(GLM)
find_package(GLFW)
find_package(GLEW)

# global includes for all projects
include_directories(${GLM_INCLUDE_DIRS})
include_directories(${GLFW_INCLUDE_DIRS})
include_directories(${GLEW_INCLUDE_DIRS})

add_subdirectory(test)
set_property(TARGET test PROPERTY FOLDER "app")

# startup project for the whole solution, if it has never been opened (wont work if refreshing)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT test)
