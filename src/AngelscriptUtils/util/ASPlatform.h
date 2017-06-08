#ifndef ANGELSCRIPT_UTIL_PLATFORM_H
#define ANGELSCRIPT_UTIL_PLATFORM_H

#include <cstdio>

#ifdef WIN32
	#if _MSC_VER < 1900
		#define snprintf _snprintf
	#endif
#endif

#ifndef MAX_PATH
//Constrain the MAX_PATH value so it can always be used on the stack.
//FILENAME_MAX is not required to be usable as an array size on some systems, so this imposes a large but still limited maximum size.
	#define FILENAME_MAX_STACK_SIZE 4096

	#if FILENAME_MAX < FILENAME_MAX_STACK_SIZE
		#define MAX_PATH FILENAME_MAX
	#else
		#define MAX_PATH FILENAME_MAX_STACK_SIZE
	#endif
#endif

/**
*	Creates a directory
*/
void MakeDirectory( const char* pszDirectoryName );

/**
*	Changes the working directory
*/
void ChangeDirectory( const char* pszDirectoryName );

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
