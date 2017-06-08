#
#   AngelscriptUtils library
#

set( PROJECT_NAME AngelscriptUtils )

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
)

add_subdirectory( add_on )
add_subdirectory( event )
add_subdirectory( ScriptAPI )
add_subdirectory( util )
add_subdirectory( wrapper )

preprocess_sources()

add_library( ${PROJECT_NAME} STATIC ${PREP_SRCS} )

target_include_directories( ${PROJECT_NAME} PRIVATE
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

set_pdb_names( ${PROJECT_NAME} )

#Create filters
create_source_groups( "${SRC_DIR}" )

install( TARGETS ${PROJECT_NAME} DESTINATION lib )

#Install the pdb files.
install_pdbs( ${PROJECT_NAME} lib )

install_includes( "${SRC_DIR}" )

clear_sources()