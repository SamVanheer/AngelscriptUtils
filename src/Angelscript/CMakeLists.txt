#
#   Angelscript library
#

set( PROJECT_NAME Angelscript )

add_subdirectory( ${EXTERNAL_DIR}/Angelscript Angelscript )

preprocess_sources()

add_library( ${PROJECT_NAME} STATIC ${PREP_SRCS} )

target_compile_definitions( ${PROJECT_NAME} PRIVATE
	${SHARED_DEFS}
)

if( NOT WIN32 )
	target_link_libraries( ${PROJECT_NAME}
		pthread
	)
endif()

set_target_properties( ${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "${LINUX_32BIT_FLAG}" LINK_FLAGS "${LINUX_32BIT_FLAG}" )

set_pdb_names( ${PROJECT_NAME} )

install( TARGETS ${PROJECT_NAME} DESTINATION lib )
install( FILES ${EXTERNAL_DIR}/Angelscript/include/angelscript.h DESTINATION include )

#Install the pdb files.
install_pdbs( ${PROJECT_NAME} lib )

#TODO: needed? - Solokiller
install_includes( "${EXTERNAL_DIR}/Angelscript/src" )

clear_sources()
