#############################################################
#                                                           #
#   AngelscriptUtils CMake build file                       #
#                                                           #
#############################################################
cmake_minimum_required(VERSION 3.16)

# Source additional modules from the "cmake" directory
list( APPEND CMAKE_MODULE_PATH
	"${CMAKE_CURRENT_LIST_DIR}/cmake"
)

include( PDBUtils )
include( Ternary )

option(UseDynamicMSVCRuntime "Use dynamic MSVC runtime" ON)

if(UseDynamicMSVCRuntime)
	set(MSVC_RUNTIME_SUFFIX "DLL")
endif()

project(AngelscriptUtils
	VERSION 2.0
	DESCRIPTION "Utility code for the Angelscript scripting library"
	LANGUAGES CXX)

#C++11 support
#Targets that use VS2013 are also supported, so C++14 cannot be used
set( CMAKE_CXX_STANDARD 11 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

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
	$<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)

set( OUTPUT_DIR "${CMAKE_SOURCE_DIR}/test" CACHE PATH "Output directory for binaries" )

#Copy all resources to the destination directory
message( STATUS "Copying resources..." )
file( COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${OUTPUT_DIR} )
message( STATUS "Done" )

#include the subdirectory containing our libs
add_subdirectory( src )

#Default to test project
set_property( DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT AngelscriptUtilsTest )
