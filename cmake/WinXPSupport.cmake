#
#	This macro checks if thread-safe initialization needs to be disabled.
#	If targeting Windows XP (v*_xp toolset) and using an MSVC version of Visual Studio 2015 or newer, it is disabled.
#	Must be called after starting a project, othewise the required variables will not have been defined yet.
macro( check_winxp_support )
	#Visual Studio only.
	if( MSVC )
		#Verify that we have the data we need.
		if( NOT MSVC_VERSION OR NOT CMAKE_VS_PLATFORM_TOOLSET )
			MESSAGE( FATAL_ERROR "Include WinXPSupport.cmake after starting a project" )
		endif()

		if( NOT "${MSVC_VERSION}" LESS 1900 AND CMAKE_VS_PLATFORM_TOOLSET MATCHES ".*_xp$" )
			MESSAGE( STATUS "Disabling Thread-safe initialization for Windows XP support" )
			#Disable thread-safe init so Windows XP users don't get crashes.
			set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:threadSafeInit-" )
		endif()
	endif()
endmacro( check_winxp_support )
