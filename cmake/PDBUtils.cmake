#	Sets PDB file names to match the binaries
macro( set_pdb_names _project_name )
	foreach( config IN LISTS CMAKE_CONFIGURATION_TYPES )
			#TODO: figure out if there's a way to check if a config is debug - Solokiller
			set( POSTFIX )
			if( ${config} STREQUAL "Debug" )
				set( POSTFIX ${CMAKE_DEBUG_POSTFIX} )
			endif()
			
			string( TOUPPER ${config} PDB_POSTFIX )
			
			set_target_properties( ${_project_name} PROPERTIES COMPILE_PDB_NAME_${PDB_POSTFIX} "${_project_name}${POSTFIX}" )
		endforeach()
endmacro( set_pdb_names )

#	Installs PDB files for a given project
macro( install_pdbs _project_name destination )
	if( WIN32 )
		foreach( config IN LISTS CMAKE_CONFIGURATION_TYPES )
			#TODO: figure out if there's a way to check if a config is debug - Solokiller
			set( POSTFIX )
			if( ${config} STREQUAL "Debug" )
				set( POSTFIX ${CMAKE_DEBUG_POSTFIX} )
			endif()
			
			install( FILES ${CMAKE_CURRENT_BINARY_DIR}/${config}/${_project_name}${POSTFIX}.pdb DESTINATION ${destination} CONFIGURATIONS ${config} )
		endforeach()
	endif()
endmacro( install_pdbs )
