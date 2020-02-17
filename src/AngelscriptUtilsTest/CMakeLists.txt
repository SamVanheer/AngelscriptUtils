add_executable(AngelscriptUtilsTest ${PREP_SRCS})

target_sources(AngelscriptUtilsTest PRIVATE
	ASCBaseEntity.h
	CBaseEntity.cpp
	CBaseEntity.h
	CScriptBaseEntity.h
	Main.cpp
	add_on/scriptany/scriptany.cpp
	add_on/scriptany/scriptany.h
	add_on/scriptarray/scriptarray.cpp
	add_on/scriptarray/scriptarray.h
	add_on/scriptdictionary/scriptdictionary.cpp
	add_on/scriptdictionary/scriptdictionary.h
	add_on/scriptstdstring/scriptstdstring.cpp
	add_on/scriptstdstring/scriptstdstring_utils.cpp
	add_on/scriptstdstring/scriptstdstring.h)
	
set_target_properties(AngelscriptUtilsTest
	PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED ON)

set_target_properties(AngelscriptUtilsTest PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>${MSVC_RUNTIME_SUFFIX}")

target_include_directories(AngelscriptUtilsTest PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(AngelscriptUtilsTest PRIVATE
	#All warnings.
	#No unknown pragma directives.
	#No attributes.
     $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
          -Wall -Wno-unknown-pragmas -Wno-attributes>
	#Warning level 4. Wall shows warnings in standard headers so we can't compile if that's enabled.
	#Treat warnings as errors.
     $<$<CXX_COMPILER_ID:MSVC>:
          /W4 /WX>)

#Create filters
get_target_property(SOURCE_FILES AngelscriptUtilsTest SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/.." FILES ${SOURCE_FILES})

# Indicate which libraries to include during the link process.
target_link_libraries(AngelscriptUtilsTest
	AngelscriptUtils)

#CMake places libraries in /Debug or /Release on Windows, so explicitly set the paths for both.
#On Linux, it uses LIBRARY_OUTPUT_DIRECTORY
set_target_properties(AngelscriptUtilsTest PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIR}"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIR}"
	RUNTIME_OUTPUT_MINSIZEREL "${OUTPUT_DIR}"
	RUNTIME_OUTPUT_RELWITHDEBINFO "${OUTPUT_DIR}")

install(TARGETS AngelscriptUtilsTest DESTINATION bin)
