add_sources(
	CASBaseEvent.h
	CASBaseEvent.cpp
	CASEvent.h
	CASEvent.cpp
	CASEventManager.h
	CASEventManager.cpp
	CASManager.h
	CASManager.cpp
	CASModuleDescriptor.h
	CASModuleDescriptor.cpp
	CASModule.h
	CASModule.cpp
	CASModuleManager.h
	CASModuleManager.cpp
	IASModuleBuilder.h
	IASInitializer.h
)

add_includes(
	CASManager.h
	CASModuleDescriptor.h
	CASModule.h
	CASModuleManager.h
	CASHook.h
	CASHookManager.h
	IASModuleBuilder.h
	IASInitializer.h
)

add_subdirectory( add_on )
add_subdirectory( ScriptAPI )
add_subdirectory( util )
add_subdirectory( wrapper )