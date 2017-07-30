#
#   Angelscript library
#

set( TARGET_NAME Angelscript )

add_subdirectory( ${EXTERNAL_DIR}/Angelscript Angelscript )

preprocess_sources()

add_library( ${TARGET_NAME} STATIC ${PREP_SRCS} )

check_winxp_support( ${TARGET_NAME} )

configure_msvc_runtime( ${TARGET_NAME} ${MSVC_RUNTIME_CONFIG} )

target_compile_definitions( ${TARGET_NAME} PRIVATE
	${SHARED_DEFS}
)

if( NOT WIN32 )
	target_link_libraries( ${TARGET_NAME}
		pthread
	)
endif()

set_target_properties( ${TARGET_NAME} PROPERTIES
	COMPILE_FLAGS "${SHARED_COMPILE_FLAGS}"
	LINK_FLAGS "${SHARED_LINK_FLAGS}"
)

set_pdb_names( ${TARGET_NAME} )

install( TARGETS ${TARGET_NAME} DESTINATION lib )
install( FILES ${EXTERNAL_DIR}/Angelscript/include/angelscript.h DESTINATION include )
install_pdbs( ${TARGET_NAME} lib )

clear_sources()
