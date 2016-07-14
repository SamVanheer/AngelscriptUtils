add_sources(
	CASManager.h
	CASManager.cpp
	CASModuleDescriptor.h
	CASModuleDescriptor.cpp
	CASModule.h
	CASModule.cpp
	CASModuleManager.h
	CASModuleManager.cpp
	CASHook.h
	CASHook.cpp
	CASHookManager.h
	CASHookManager.cpp
	IASModuleBuilder.h
)

add_subdirectory( add_on )
add_subdirectory( ScriptAPI )
add_subdirectory( util )
add_subdirectory( wrapper )