cmake_minimum_required(VERSION 3.16)

# Source additional modules from the "cmake" directory
list(APPEND CMAKE_MODULE_PATH
	"${CMAKE_CURRENT_LIST_DIR}/cmake")

include(PDBUtils)

if (MSVC)
	option(UseDynamicMSVCRuntime "Use dynamic MSVC runtime" ON)
endif()

if(NOT MSVC OR UseDynamicMSVCRuntime)
	set(MSVC_RUNTIME_SUFFIX "DLL")
endif()

project(AngelscriptUtils
	VERSION 2.0
	DESCRIPTION "Utility code for the Angelscript scripting library"
	LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Debug builds should have a distinct name.
if (NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX d)
endif()

#include the subdirectory containing our libs
add_subdirectory(src)

#Default to test project
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT AngelscriptUtilsTest)
