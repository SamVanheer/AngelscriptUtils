cmake_minimum_required( VERSION 3.6 )

# function to collect all the sources from sub-directories
# into a single list
function(add_sources)
  get_property(is_defined GLOBAL PROPERTY SRCS_LIST DEFINED)
  if(NOT is_defined)
    define_property(GLOBAL PROPERTY SRCS_LIST
      BRIEF_DOCS "List of source files"
      FULL_DOCS "List of source files to be compiled in one library")
  endif()
  # make absolute paths
  set(SRCS)
  foreach(s IN LISTS ARGN)
    if(NOT IS_ABSOLUTE "${s}")
      get_filename_component(s "${s}" ABSOLUTE)
    endif()
    list(APPEND SRCS "${s}")
  endforeach()
  # append to global list
  set_property(GLOBAL APPEND PROPERTY SRCS_LIST "${SRCS}")
endfunction(add_sources)

# preprocess sources
function( preprocess_sources )
	set(PREP_SRCS PARENT_SCOPE)
	get_property(SRCS GLOBAL PROPERTY SRCS_LIST)
	foreach(s IN LISTS SRCS)
	  file(RELATIVE_PATH rs "${CMAKE_CURRENT_SOURCE_DIR}" "${s}")
	  string(REGEX REPLACE "r$" "" o "${CMAKE_CURRENT_BINARY_DIR}/${rs}")
	  add_custom_command(
		OUTPUT "${o}"
		COMMAND ${CMAKE_COMMAND} -E copy "${s}" "${o}"
		DEPENDS "${s}"
		COMMENT "Creating ${o}"
		VERBATIM
		)
	  list(APPEND PREP_SRCS "${s}")
	endforeach()
	set(PREP_SRCS ${PREP_SRCS} PARENT_SCOPE)
endfunction( preprocess_sources )

function( create_source_groups _src_root_path )
	get_property(SRCS GLOBAL PROPERTY SRCS_LIST)
	foreach(_source IN ITEMS ${SRCS})
		get_filename_component(_source_path "${_source}" PATH)
		file(RELATIVE_PATH _source_path_rel "${_src_root_path}" "${_source_path}")
		string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
		source_group("${_group_path}" FILES "${_source}")
	endforeach()
endfunction( create_source_groups )

#Function to clear the sources list.
function( clear_sources )
	set_property( GLOBAL PROPERTY SRCS_LIST "" )
	set( PREP_SRCS PARENT_SCOPE )
endfunction( clear_sources )

# Function to add include files.
function( add_includes )
  get_property(is_defined GLOBAL PROPERTY INCLUDES_LIST DEFINED)
  if(NOT is_defined)
    define_property(GLOBAL PROPERTY INCLUDES_LIST
      BRIEF_DOCS "List of include files"
      FULL_DOCS "List of include files to be compiled in one library")
  endif()
  # make absolute paths
  set(INCLUDES)
  foreach(s IN LISTS ARGN)
    if(NOT IS_ABSOLUTE "${s}")
      get_filename_component(s "${s}" ABSOLUTE)
    endif()
    list(APPEND INCLUDES "${s}")
  endforeach()
  # append to global list
  set_property(GLOBAL APPEND PROPERTY INCLUDES_LIST "${INCLUDES}")
endfunction( add_includes )

# Function to install includes.
function( install_includes _include_root_path )
	get_property(INCLUDES GLOBAL PROPERTY INCLUDES_LIST)
	
	foreach(_include IN ITEMS ${INCLUDES})
		get_filename_component(_include_path "${_include}" PATH)
		file(RELATIVE_PATH _include_path_rel "${_include_root_path}" "${_include_path}")
		string(REPLACE "/" "\\" _group_path "${_include_path_rel}")
		install( FILES "${_include}" DESTINATION "include/${_group_path}" )
	endforeach()
	
	set_property( GLOBAL PROPERTY INCLUDES_LIST "" )
endfunction( install_includes )

macro(configure_msvc_runtime)
  if(MSVC)
    # Default to statically-linked runtime.
    if("${MSVC_RUNTIME}" STREQUAL "")
      set(MSVC_RUNTIME "static")
    endif()
    # Set compiler options.
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
    if(${MSVC_RUNTIME} STREQUAL "static")
      message(STATUS
        "MSVC -> forcing use of statically-linked runtime."
      )
      foreach(variable ${variables})
        if(${variable} MATCHES "/MD")
          string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
        endif()
      endforeach()
    else()
      message(STATUS
        "MSVC -> forcing use of dynamically-linked runtime."
      )
      foreach(variable ${variables})
        if(${variable} MATCHES "/MT")
          string(REGEX REPLACE "/MT" "/MD" ${variable} "${${variable}}")
        endif()
      endforeach()
    endif()
  endif()
endmacro()

project( AngelscriptUtils )

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

install( TARGETS Angelscript DESTINATION lib )
install( FILES external/Angelscript/include/angelscript.h DESTINATION include )
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

set_target_properties( AngelscriptUtils PROPERTIES COMPILE_FLAGS "${LINUX_32BIT_FLAG}" LINK_FLAGS "${LINUX_32BIT_FLAG}" )

#Create filters
create_source_groups( "${CMAKE_SOURCE_DIR}/src" )

install( TARGETS AngelscriptUtils DESTINATION lib )
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

set_target_properties( AngelscriptUtilsTest PROPERTIES COMPILE_FLAGS "${LINUX_32BIT_FLAG}" LINK_FLAGS "${LINUX_32BIT_FLAG}" )

#Create filters
create_source_groups( "${CMAKE_SOURCE_DIR}/src" )

set_property( GLOBAL PROPERTY VS_STARTUP_PROJECT AngelscriptUtilsTest )

# Indicate which libraries to include during the link process.
target_link_libraries( AngelscriptUtilsTest AngelscriptUtils Angelscript )

install( TARGETS AngelscriptUtilsTest DESTINATION bin )
install_includes( "${CMAKE_SOURCE_DIR}/src" )

clear_sources()