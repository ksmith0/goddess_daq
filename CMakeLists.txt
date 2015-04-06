cmake_minimum_required (VERSION 2.8 FATAL_ERROR)
project (goddess_daq)

#Specify the install prefix as the exec directory unless specified
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	message(STATUS "Install Prefix not specified.")
	file(MAKE_DIRECTORY exec)
	get_filename_component(INSTALL_DIR ${CMAKE_SOURCE_DIR}/exec REALPATH)
	set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR} CACHE PATH
			"Install Prefix" FORCE)
endif ()
message(STATUS "Installing to ${CMAKE_INSTALL_PREFIX}")

#Define the default build type
IF(NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Release CACHE STRING
		"Build type, options are: None Debug Release
			RelWithDebInfo MinSizeRel."
		FORCE)
	message(STATUS "Build type not defined, using default.")
ENDIF(NOT CMAKE_BUILD_TYPE)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

#Add custom modules to module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

#Find the ROOT package and configure the flags and paths
find_package (ROOT REQUIRED)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ROOT_CXX_FLAGS}")
include_directories(${ROOT_INCLUDE_DIR})
link_directories(${ROOT_LIBRARY_DIR})

#Find the zlib fro compression
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

add_definitions(-DLINUX -D_FILE_OFFSET_BITS=64)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")

#Include our headers and continue in the source directory
include_directories(include)
add_subdirectory(source)