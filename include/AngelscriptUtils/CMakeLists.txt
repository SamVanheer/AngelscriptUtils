target_sources(AngelscriptUtils
	PRIVATE
		ASUtilsConfig.h
		CASManager.h
		CASModuleDescriptor.h
		CASModule.h
		CASModuleManager.h
		IASInitializer.h
		IASModuleBuilder.h
		std_make_unique.h
		add_on/scriptbuilder/scriptbuilder.h)

add_subdirectory( event )
add_subdirectory( ScriptAPI )
add_subdirectory( util )
add_subdirectory(utility)
add_subdirectory( wrapper )
