#ifndef ANGELSCRIPT_UTIL_PLATFORM_H
#define ANGELSCRIPT_UTIL_PLATFORM_H

#ifdef WIN32
//Some projects might manage their Windows dependencies differenly.
#ifndef ASUTILS_DISABLE_WINDOWS_INCLUDES
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <direct.h>
#endif

#if _MSC_VER < 1900
#define snprintf _snprintf
#endif

#define mkdir _mkdir
#define chdir _chdir

#ifndef ALLOW_VOID_DEF
#undef VOID
#endif

#define MakeDirectory( pszDirectory ) CreateDirectoryA( pszDirectory, nullptr )

#else
#include <linux/limits.h>
#include <unistd.h>

#define MAX_PATH PATH_MAX

#define MakeDirectory( pszDirectory ) mkdir( pszDirectory, 0777 )
#endif

/**
*	Used to mark function parameters as unused.
*/
#define ASUNREFERENCED( x )

/**
*	Used to mark function parameters as unused. Used at the start of the function body.
*/
#define ASREFERENCED( x ) ( ( x ) = ( x ) )

template<typename T, size_t SIZE>
inline size_t _ASArraySizeof( const T( & )[ SIZE ] )
{
	return SIZE;
}

#define ASARRAYSIZE( p )	_ASArraySizeof( p )

#endif //ANGELSCRIPT_UTIL_PLATFORM_H
