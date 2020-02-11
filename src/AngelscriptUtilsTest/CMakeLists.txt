#
#   AngelscriptUtils Test project
#

set( TARGET_NAME AngelscriptUtilsTest )

set(ROOT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable( ${TARGET_NAME} ${PREP_SRCS} )

target_sources(${TARGET_NAME} PRIVATE
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

set_target_properties(${TARGET_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>${MSVC_RUNTIME_SUFFIX}")

target_include_directories( ${TARGET_NAME} PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${ROOT_SOURCE_DIR}
)

set_target_properties( ${TARGET_NAME} PROPERTIES
	COMPILE_FLAGS "${SHARED_COMPILE_FLAGS}"
	LINK_FLAGS "${SHARED_LINK_FLAGS}"
)

#Create filters
get_target_property(SOURCE_FILES ${TARGET_NAME} SOURCES)
source_group(TREE ${ROOT_SOURCE_DIR} FILES ${SOURCE_FILES})

# Indicate which libraries to include during the link process.
target_link_libraries( ${TARGET_NAME}
	AngelscriptUtils
)

#CMake places libraries in /Debug or /Release on Windows, so explicitly set the paths for both.
#On Linux, it uses LIBRARY_OUTPUT_DIRECTORY
set_target_properties( ${TARGET_NAME} PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIR}"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIR}"
	RUNTIME_OUTPUT_MINSIZEREL "${OUTPUT_DIR}"
	RUNTIME_OUTPUT_RELWITHDEBINFO "${OUTPUT_DIR}"
)

install( TARGETS ${TARGET_NAME} DESTINATION bin )
