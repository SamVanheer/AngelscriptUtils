#	Installs PDB files for a given project
macro( install_pdbs _project_name destination )
	if( WIN32 )
		foreach( config in LISTS CMAKE_CONFIGURATION_TYPES )
			#TODO: figure out if there's a way to check if a config is debug - Solokiller
			set( POSTFIX )
			if( ${config} STREQUAL "Debug" )
				set( POSTFIX ${CMAKE_DEBUG_POSTFIX} )
			endif()
			
			install( FILES ${CMAKE_CURRENT_BINARY_DIR}/${config}/${_project_name}${POSTFIX}.pdb DESTINATION ${destination} CONFIGURATIONS ${config} )
		endforeach()
	endif()
endmacro( install_pdbs )
