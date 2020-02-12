target_sources(AngelscriptUtils
	PRIVATE
		ASUtilsConfig.h
		CASLoggingContextResultHandler.h
		CASManager.h
		CASModuleDescriptor.h
		CASModule.h
		CASModuleManager.h
		IASContextResultHandler.h
		IASInitializer.h
		IASModuleBuilder.h
		std_make_unique.h
		add_on/scriptbuilder/scriptbuilder.h)

add_subdirectory( event )
add_subdirectory( ScriptAPI )
add_subdirectory( util )
add_subdirectory( wrapper )
