#
#   AngelscriptUtils library
#

set( TARGET_NAME AngelscriptUtils )

find_package( Threads REQUIRED )
find_package( Angelscript CONFIG 2.32.0 REQUIRED )

add_sources(
	CASLoggingContextResultHandler.cpp
	CASLoggingContextResultHandler.h
	CASManager.cpp
	CASManager.h
	CASModuleDescriptor.cpp
	CASModuleDescriptor.h
	CASModule.cpp
	CASModule.h
	CASModuleManager.cpp
	CASModuleManager.h
	IASContextResultHandler.h
	IASInitializer.h
	IASModuleBuilder.h
	add_on/scriptbuilder/scriptbuilder.cpp
)

add_includes(
	CASLoggingContextResultHandler.h
	CASManager.h
	CASModuleDescriptor.h
	CASModule.h
	CASModuleManager.h
	IASContextResultHandler.h
	IASInitializer.h
	IASModuleBuilder.h
	std_make_unique.h
	add_on/scriptbuilder/scriptbuilder.h
)

add_subdirectory( event )
add_subdirectory( ScriptAPI )
add_subdirectory( util )
add_subdirectory( wrapper )

set( CASMODULE_USER_DATA_ID "10001" CACHE STRING "Value for the CASModule user data ID" )

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/ASUtilsConfig.h.in
	${CMAKE_CURRENT_BINARY_DIR}/ASUtilsConfig.h
	@ONLY
)

preprocess_sources()

add_library( ${TARGET_NAME} STATIC ${PREP_SRCS} )

check_winxp_support( ${TARGET_NAME} )

configure_msvc_runtime( ${TARGET_NAME} ${MSVC_RUNTIME_CONFIG} )

# The config file is in the binary dir, so include the path
target_include_directories( ${TARGET_NAME} PUBLIC
	${CMAKE_CURRENT_BINARY_DIR}
)

target_include_directories( ${TARGET_NAME} PRIVATE
	${SRC_DIR}
)

target_compile_definitions( ${TARGET_NAME} PRIVATE
	${SHARED_DEFS}
)

target_link_libraries( ${TARGET_NAME}
	Angelscript::angelscript
)

set_target_properties( ${TARGET_NAME} PROPERTIES
	COMPILE_FLAGS "${SHARED_COMPILE_FLAGS}"
	LINK_FLAGS "${SHARED_LINK_FLAGS}"
)

set_pdb_names( ${TARGET_NAME} )

#Create filters
create_source_groups( "${SRC_DIR}" )

install( TARGETS ${TARGET_NAME} DESTINATION lib )
install_pdbs( ${TARGET_NAME} lib )
install_includes( "${SRC_DIR}" )

clear_sources()
