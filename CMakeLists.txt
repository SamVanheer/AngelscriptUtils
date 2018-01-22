#############################################################
#                                                           #
#   AngelscriptUtils CMake build file                       #
#                                                           #
#############################################################
cmake_minimum_required( VERSION 3.6 )

# Source additional modules from the "cmake" directory
list( APPEND CMAKE_MODULE_PATH
	"${CMAKE_CURRENT_LIST_DIR}/cmake"
)

include( InputFilesList )
include( MSVCRuntime )
include( PDBUtils )
include( WinXPSupport )
include( Ternary )

project( AngelscriptUtils LANGUAGES CXX )

set( MSVC_RUNTIME "dynamic" CACHE STRING "Runtime to use for MSVC" )
set_property( CACHE MSVC_RUNTIME PROPERTY STRINGS dynamic static )

string( TOUPPER ${MSVC_RUNTIME} MSVC_RUNTIME_CONFIG )

#C++11 support
#Targets that use VS2013 are also supported, so C++14 cannot be used
set( CMAKE_CXX_STANDARD 11 )

#Debug builds should have a distinct name.
if( NOT CMAKE_DEBUG_POSTFIX )
  set( CMAKE_DEBUG_POSTFIX d )
endif()

ternary( LINUX_32BIT_FLAG UNIX "-m32" "" )

set( SHARED_COMPILE_FLAGS
	${LINUX_32BIT_FLAG}
)

#Set global compiler settings.
if( MSVC )
	#Warning level 4. Wall shows warnings in standard headers so we can't compile if that's enabled.
	#Treat warnings as errors.
	set( SHARED_COMPILE_FLAGS
		"${SHARED_COMPILE_FLAGS} /W4 /WX"
	)
else()
	#All warnings.
	#No unknown pragma directives.
	#No attributes.
	set( SHARED_COMPILE_FLAGS
		"${SHARED_COMPILE_FLAGS} -Wall -Wno-unknown-pragmas -Wno-attributes"
	)
endif()

set( SHARED_LINK_FLAGS
	${LINUX_32BIT_FLAG}
)

set( SHARED_DEFS
)

if( MSVC )
	set( SHARED_DEFS
		${SHARED_DEFS}
		_CRT_SECURE_NO_WARNINGS
	)
endif()

if( AS_NO_EXCEPTIONS )
	set( SHARED_DEFS
		${SHARED_DEFS}
		AS_NO_EXCEPTIONS
	)
endif()

set( OUTPUT_DIR "${CMAKE_SOURCE_DIR}/test" CACHE PATH "Output directory for binaries" )

set( EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/external" )
set( SRC_DIR "${CMAKE_SOURCE_DIR}/src" )

#Copy all resources to the destination directory
message( STATUS "Copying resources..." )
file( COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${OUTPUT_DIR} )
message( STATUS "Done" )

#include the subdirectory containing our libs
add_subdirectory( src )

#Default to test project
set_property( DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT AngelscriptUtilsTest )
