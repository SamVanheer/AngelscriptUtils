cmake_minimum_required( VERSION 3.6 )

include( cmake/InputFilesList.cmake )
include( cmake/MSVCRuntime.cmake )
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

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y" )

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

#Debug builds should have a distinct name.
if( NOT CMAKE_DEBUG_POSTFIX )
  set( CMAKE_DEBUG_POSTFIX d )
endif()

# Angelscript library

set( AS_STRING_OBJNAME "string" CACHE STRING "The name of the object type used to represent strings" )

add_definitions( -DAS_NO_EXCEPTIONS )

add_subdirectory( external/Angelscript )

preprocess_sources()

add_library( Angelscript STATIC ${PREP_SRCS} )

if( NOT WIN32 )
	target_link_libraries( Angelscript
		pthread
	)
endif()

set_target_properties( Angelscript PROPERTIES COMPILE_FLAGS "${LINUX_32BIT_FLAG}" LINK_FLAGS "${LINUX_32BIT_FLAG}" )

#Properly named pdb. By default it always uses the base name.
set_target_properties( Angelscript PROPERTIES COMPILE_PDB_NAME_DEBUG "Angelscriptd" )
#Match the debug version so its relative position is the same.
set_target_properties( Angelscript PROPERTIES COMPILE_PDB_NAME_RELWITHDEBINFO "Angelscript" )

install( TARGETS Angelscript DESTINATION lib )
install( FILES external/Angelscript/include/angelscript.h DESTINATION include )

#Install the pdb files.
if( WIN32 )
	install( FILES ${CMAKE_BINARY_DIR}/Debug/Angelscriptd.pdb DESTINATION lib CONFIGURATIONS Debug )
	install( FILES ${CMAKE_BINARY_DIR}/RelWithDebInfo/Angelscript.pdb DESTINATION lib CONFIGURATIONS RelWithDebInfo )
endif()

install_includes( "${CMAKE_SOURCE_DIR}/external/Angelscript/src" )

clear_sources()

#version number
set( AngelscriptUtils_VERSION_MAJOR 1 )
set( AngelscriptUtils_VERSION_MINOR 0 )

add_definitions( -D_CRT_SECURE_NO_WARNINGS )

#include the subdirectory containing our libs
add_subdirectory( src )

include_directories( src external/Angelscript/include )

preprocess_sources()

add_library( AngelscriptUtils STATIC ${PREP_SRCS} )

target_compile_definitions( AngelscriptUtils PRIVATE
	AS_STRING_OBJNAME="${AS_STRING_OBJNAME}"
)

set_target_properties( AngelscriptUtils PROPERTIES COMPILE_FLAGS "${LINUX_32BIT_FLAG}" LINK_FLAGS "${LINUX_32BIT_FLAG}" )

#Properly named pdb. By default it always uses the base name.
set_target_properties( AngelscriptUtils PROPERTIES COMPILE_PDB_NAME_DEBUG "AngelscriptUtilsd" )
#Match the debug version so its relative position is the same.
set_target_properties( AngelscriptUtils PROPERTIES COMPILE_PDB_NAME_RELWITHDEBINFO "AngelscriptUtils" )

#Create filters
create_source_groups( "${CMAKE_SOURCE_DIR}/src" )

install( TARGETS AngelscriptUtils DESTINATION lib )

#Install the pdb files.
if( WIN32 )
	install( FILES ${CMAKE_BINARY_DIR}/Debug/AngelscriptUtilsd.pdb DESTINATION lib CONFIGURATIONS Debug )
	install( FILES ${CMAKE_BINARY_DIR}/RelWithDebInfo/AngelscriptUtils.pdb DESTINATION lib CONFIGURATIONS RelWithDebInfo )
endif()

install_includes( "${CMAKE_SOURCE_DIR}/src" )

clear_sources()

#This really needs a better approach.
add_sources( 
	src/test/CBaseEntity.h
	src/test/CBaseEntity.cpp
	src/test/CScriptBaseEntity.h
	src/test/ASCBaseEntity.h
	src/test/Main.cpp
)

preprocess_sources()

#indicate the entry point for the executable
add_executable( AngelscriptUtilsTest ${PREP_SRCS} )

target_compile_definitions( AngelscriptUtilsTest PRIVATE
	AS_STRING_OBJNAME="${AS_STRING_OBJNAME}"
)

set_target_properties( AngelscriptUtilsTest PROPERTIES COMPILE_FLAGS "${LINUX_32BIT_FLAG}" LINK_FLAGS "${LINUX_32BIT_FLAG}" )

#Create filters
create_source_groups( "${CMAKE_SOURCE_DIR}/src" )

set_property( GLOBAL PROPERTY VS_STARTUP_PROJECT AngelscriptUtilsTest )

# Indicate which libraries to include during the link process.
target_link_libraries( AngelscriptUtilsTest AngelscriptUtils Angelscript )

install( TARGETS AngelscriptUtilsTest DESTINATION bin )
install_includes( "${CMAKE_SOURCE_DIR}/src" )

clear_sources()