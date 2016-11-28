#ifndef ANGELSCRIPT_UTIL_PLATFORM_H
#define ANGELSCRIPT_UTIL_PLATFORM_H

#ifdef WIN32
//Some projects might manage their Windows dependencies differenly.
#ifndef ASUTILS_DISABLE_WINDOWS_INCLUDES
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#if _MSC_VER < 1900
#define snprintf _snprintf
#endif

#define mkdir _mkdir

#ifndef ALLOW_VOID_DEF
#undef VOID
#endif

#define MakeDirectory( pszDirectory ) CreateDirectoryA( pszDirectory, nullptr )

#else
#include <linux/limits.h>

#define MAX_PATH PATH_MAX

#define MakeDirectory( pszDirectory ) mkdir( pszDirectory, 0777 )
#endif

/**
*	Used to mark function parameters as unused.
*/
#define ASUNREFERENCED( x )

#endif //ANGELSCRIPT_UTIL_PLATFORM_H
