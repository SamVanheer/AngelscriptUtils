target_sources(AngelscriptUtils
	PRIVATE
		add_on/scriptbuilder/scriptbuilder.h)

add_subdirectory(compilation)
add_subdirectory(event)
add_subdirectory(execution)
add_subdirectory(ScriptAPI)
add_subdirectory(utility)
add_subdirectory(wrapper)
