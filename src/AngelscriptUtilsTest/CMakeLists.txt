#
#   AngelscriptUtils Test project
#

set( TARGET_NAME AngelscriptUtilsTest )

add_sources( 
	ASCBaseEntity.h
	CBaseEntity.cpp
	CBaseEntity.h
	CScriptBaseEntity.h
	Main.cpp
)

preprocess_sources()

add_executable( ${TARGET_NAME} ${PREP_SRCS} )

check_winxp_support( ${TARGET_NAME} )

configure_msvc_runtime( ${TARGET_NAME} ${MSVC_RUNTIME_CONFIG} )

target_include_directories( ${TARGET_NAME} PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${SRC_DIR}
	${EXTERNAL_DIR}/Angelscript/include
)

target_compile_definitions( ${TARGET_NAME} PRIVATE
	AS_STRING_OBJNAME="${AS_STRING_OBJNAME}"
	${SHARED_DEFS}
)

set_target_properties( ${TARGET_NAME} PROPERTIES
	COMPILE_FLAGS "${SHARED_COMPILE_FLAGS}"
	LINK_FLAGS "${SHARED_LINK_FLAGS}"
)

#Create filters
create_source_groups( "${SRC_DIR}" )

# Indicate which libraries to include during the link process.
target_link_libraries( ${TARGET_NAME}
	AngelscriptUtils
	Angelscript
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
install_includes( "${SRC_DIR}" )

clear_sources()
