add_sources(
	CASBaseEvent.h
	CASBaseEvent.cpp
	CASBaseEventCaller.h
	CASEvent.h
	CASEvent.cpp
	CASEventCaller.h
	CASEventCaller.cpp
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
	CASBaseEvent.h
	CASBaseEventCaller.h
	CASEvent.h
	CASEventCaller.h
	CASEventManager.h
	CASManager.h
	CASModuleDescriptor.h
	CASModule.h
	CASModuleManager.h
	IASModuleBuilder.h
	IASInitializer.h
)

add_subdirectory( add_on )
add_subdirectory( ScriptAPI )
add_subdirectory( util )
add_subdirectory( wrapper )