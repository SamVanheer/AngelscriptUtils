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

#Properly named pdb. By default it always uses the base name.
set_target_properties( ${PROJECT_NAME} PROPERTIES COMPILE_PDB_NAME_DEBUG "${PROJECT_NAME}${CMAKE_DEBUG_POSTFIX}" )
#Match the debug version so its relative position is the same.
set_target_properties( ${PROJECT_NAME} PROPERTIES COMPILE_PDB_NAME_RELWITHDEBINFO "${PROJECT_NAME}" )

install( TARGETS ${PROJECT_NAME} DESTINATION lib )
install( FILES ${EXTERNAL_DIR}/Angelscript/include/angelscript.h DESTINATION include )

#Install the pdb files.
if( WIN32 )
	install( FILES ${CMAKE_BINARY_DIR}/Debug/${PROJECT_NAME}${CMAKE_DEBUG_POSTFIX}.pdb DESTINATION lib CONFIGURATIONS Debug )
	install( FILES ${CMAKE_BINARY_DIR}/RelWithDebInfo/${PROJECT_NAME}.pdb DESTINATION lib CONFIGURATIONS RelWithDebInfo )
endif()

#TODO: needed? - Solokiller
install_includes( "${EXTERNAL_DIR}/Angelscript/src" )

clear_sources()
