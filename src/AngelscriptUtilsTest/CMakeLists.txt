#
#   AngelscriptUtils Test project
#

set( PROJECT_NAME AngelscriptUtilsTest )

add_sources( 
	ASCBaseEntity.h
	CBaseEntity.cpp
	CBaseEntity.h
	CScriptBaseEntity.h
	Main.cpp
)

preprocess_sources()

#indicate the entry point for the executable
add_executable( ${PROJECT_NAME} ${PREP_SRCS} )

target_include_directories( ${PROJECT_NAME} PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${SRC_DIR}
	${EXTERNAL_DIR}/Angelscript/include
)

target_compile_definitions( ${PROJECT_NAME} PRIVATE
	AS_STRING_OBJNAME="${AS_STRING_OBJNAME}"
	${SHARED_DEFS}
)

set_target_properties( ${PROJECT_NAME} PROPERTIES
	COMPILE_FLAGS "${LINUX_32BIT_FLAG}"
	LINK_FLAGS "${LINUX_32BIT_FLAG}"
)

#Create filters
create_source_groups( "${SRC_DIR}" )

set_property( GLOBAL PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME} )

# Indicate which libraries to include during the link process.
target_link_libraries( ${PROJECT_NAME}
	AngelscriptUtils
	Angelscript
)

#CMake places libraries in /Debug or /Release on Windows, so explicitly set the paths for both.
#On Linux, it uses LIBRARY_OUTPUT_DIRECTORY
set_target_properties( ${PROJECT_NAME} PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIR}"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIR}"
	RUNTIME_OUTPUT_MINSIZEREL "${OUTPUT_DIR}"
	RUNTIME_OUTPUT_RELWITHDEBINFO "${OUTPUT_DIR}"
)

install( TARGETS ${PROJECT_NAME} DESTINATION bin )
install_includes( "${SRC_DIR}" )

clear_sources()
