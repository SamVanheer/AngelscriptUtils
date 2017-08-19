#
#   AngelscriptUtils library
#

set( TARGET_NAME AngelscriptUtils )

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
)

add_subdirectory( add_on )
add_subdirectory( event )
add_subdirectory( ScriptAPI )
add_subdirectory( util )
add_subdirectory( wrapper )

preprocess_sources()

add_library( ${TARGET_NAME} STATIC ${PREP_SRCS} )

check_winxp_support( ${TARGET_NAME} )

configure_msvc_runtime( ${TARGET_NAME} ${MSVC_RUNTIME_CONFIG} )

target_include_directories( ${TARGET_NAME} PRIVATE
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

set_pdb_names( ${TARGET_NAME} )

#Create filters
create_source_groups( "${SRC_DIR}" )

install( TARGETS ${TARGET_NAME} DESTINATION lib )
install_pdbs( ${TARGET_NAME} lib )
install_includes( "${SRC_DIR}" )

clear_sources()
