add_sources(
	CASLoggingContextResultHandler.h
	CASLoggingContextResultHandler.cpp
	CASManager.h
	CASManager.cpp
	CASModuleDescriptor.h
	CASModuleDescriptor.cpp
	CASModule.h
	CASModule.cpp
	CASModuleManager.h
	CASModuleManager.cpp
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
