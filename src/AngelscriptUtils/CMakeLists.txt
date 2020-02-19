set(ANGELSCRIPTUTILS_VERSION_MAJOR 0)
set(ANGELSCRIPTUTILS_VERSION_MINOR 1)
set(ANGELSCRIPTUTILS_VERSION_PATCH 0)
set(PROJECT_VERSION ${ANGELSCRIPTUTILS_VERSION_MAJOR}.${ANGELSCRIPTUTILS_VERSION_MINOR}.${ANGELSCRIPTUTILS_VERSION_PATCH})

find_package(Threads REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Angelscript CONFIG 2.34.0 REQUIRED)

set(ASUTILS_CASMODULE_USER_DATA_ID "10001" CACHE STRING "Value for the CASModule user data ID")

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

configure_file(
	${INCLUDE_DIR}/AngelscriptUtils/ASUtilsConfig.h.in
	${INCLUDE_DIR}/AngelscriptUtils/ASUtilsConfig.h
	@ONLY)

add_library(AngelscriptUtils STATIC)

set_target_properties(AngelscriptUtils
	PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED ON)

set_target_properties(AngelscriptUtils PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>${MSVC_RUNTIME_SUFFIX}")

target_include_directories(AngelscriptUtils PUBLIC
	"$<BUILD_INTERFACE:${INCLUDE_DIR}>"
	"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

target_link_libraries(AngelscriptUtils
	Angelscript::angelscript
	spdlog::spdlog)

target_compile_options(AngelscriptUtils PRIVATE
	#All warnings.
	#No unknown pragma directives.
	#No attributes.
     $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
          -Wall -Wno-unknown-pragmas -Wno-attributes>
	#Warning level 4. Wall shows warnings in standard headers so we can't compile if that's enabled.
	#Treat warnings as errors.
     $<$<CXX_COMPILER_ID:MSVC>:
          /W4 /WX>)

target_sources(AngelscriptUtils
	PRIVATE
		CASManager.cpp
		CASModuleDescriptor.cpp
		CASModule.cpp
		CASModuleManager.cpp
		add_on/scriptbuilder/scriptbuilder.cpp)

add_subdirectory(event)
add_subdirectory(ScriptAPI)
add_subdirectory(utility)
add_subdirectory(wrapper)

get_target_property(SOURCE_FILES AngelscriptUtils SOURCES)

add_subdirectory(${INCLUDE_DIR}/AngelscriptUtils headers)

set_pdb_names(AngelscriptUtils)

#Create filters
get_target_property(HEADER_FILES AngelscriptUtils SOURCES)

foreach (header in LISTS ${SOURCE_FILES})
	list(REMOVE_ITEM HEADER_FILES ${header})
endforeach()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/.." FILES ${SOURCE_FILES})
source_group(TREE ${INCLUDE_DIR} FILES ${HEADER_FILES})

install_pdbs(AngelscriptUtils lib)

#See https://cmake.org/cmake/help/latest/manual/cmake-packages.7.html#creating-packages for a detailed explanation about this part
install(TARGETS AngelscriptUtils EXPORT AngelscriptUtilsTargets
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	INCLUDES DESTINATION include)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}../../include DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/AngelscriptUtils/AngelscriptUtilsConfigVersion.cmake"
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY AnyNewerVersion)

export(EXPORT AngelscriptUtilsTargets
	FILE "${CMAKE_CURRENT_BINARY_DIR}/AngelscriptUtils/AngelscriptUtilsTargets.cmake"
	NAMESPACE AngelscriptUtils::)
	
configure_file(cmake/AngelscriptUtilsConfig.cmake
	"${CMAKE_CURRENT_BINARY_DIR}/AngelscriptUtils/AngelscriptUtilsConfig.cmake"
	COPYONLY)

set( ConfigPackageLocation lib/cmake/AngelscriptUtils)

install(EXPORT AngelscriptUtilsTargets
	FILE AngelscriptUtilsTargets.cmake
	NAMESPACE AngelscriptUtils::
	DESTINATION ${ConfigPackageLocation})
	
install(
	FILES
		cmake/AngelscriptUtilsConfig.cmake
		"${CMAKE_CURRENT_BINARY_DIR}/AngelscriptUtils/AngelscriptUtilsConfigVersion.cmake"
	DESTINATION ${ConfigPackageLocation}
	COMPONENT Devel)
