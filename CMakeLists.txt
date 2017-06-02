cmake_minimum_required( VERSION 3.6 )

include( cmake/InputFilesList.cmake )
include( cmake/MSVCRuntime.cmake )
include( cmake/PDBUtils.cmake )
include( cmake/WinXPSupport.cmake )

project( AngelscriptUtils )

check_winxp_support()

configure_msvc_runtime()

set(variables
  CMAKE_C_FLAGS_DEBUG
  CMAKE_C_FLAGS_MINSIZEREL
  CMAKE_C_FLAGS_RELEASE
  CMAKE_C_FLAGS_RELWITHDEBINFO
  CMAKE_CXX_FLAGS_DEBUG
  CMAKE_CXX_FLAGS_MINSIZEREL
  CMAKE_CXX_FLAGS_RELEASE
  CMAKE_CXX_FLAGS_RELWITHDEBINFO
)
message(STATUS "Initial build flags:")
foreach(variable ${variables})
  message(STATUS "  '${variable}': ${${variable}}")
endforeach()
message(STATUS "")

#C++14 support
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y" )

#Debug builds should have a distinct name.
if( NOT CMAKE_DEBUG_POSTFIX )
  set( CMAKE_DEBUG_POSTFIX d )
endif()

#Set global compiler settings.
if( MSVC )
	#Warning level 4. Wall shows warnings in standard headers so we can't compile if that's enabled.
	#Treat warnings as errors.
	set( GLOBAL_COMPILER_FLAGS "/W4 /WX" )
else()
	#All warnings.
	#No unknown pragma directives.
	#No attributes.
	set( GLOBAL_COMPILER_FLAGS "-Wall -Wno-unknown-pragmas -Wno-attributes" )
endif()

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GLOBAL_COMPILER_FLAGS}" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GLOBAL_COMPILER_FLAGS}" )

if( UNIX )
	set( LINUX_32BIT_FLAG "-m32" )
else()
	set( LINUX_32BIT_FLAG "" )
endif()

set( AS_NO_EXCEPTIONS OFF CACHE BOOL "Whether to disable exception handling in the Angelscript library" )

set( SHARED_DEFS
	_CRT_SECURE_NO_WARNINGS
)

if( AS_NO_EXCEPTIONS )
	set( SHARED_DEFS
		${SHARED_DEFS}
		AS_NO_EXCEPTIONS
	)
endif()

set( AS_STRING_OBJNAME "string" CACHE STRING "The name of the object type used to represent strings" )

#version number
set( AngelscriptUtils_VERSION_MAJOR 1 )
set( AngelscriptUtils_VERSION_MINOR 0 )

set( EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/external" )
set( SRC_DIR "${CMAKE_SOURCE_DIR}/src" )

#include the subdirectory containing our libs
add_subdirectory( src )

#Default to test project
set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT AngelscriptUtilsTest )
