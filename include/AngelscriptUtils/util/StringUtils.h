#ifndef ANGELSCRIPT_UTIL_STRINGUTILS_H
#define ANGELSCRIPT_UTIL_STRINGUTILS_H

#include <algorithm>
#include <cctype>
#include <functional>
#include <string>

//Implemented in the as namespace to prevent collisions with other implementations.
namespace as
{
/*
*	This code is based on the following Stack Overflow answer: http://stackoverflow.com/a/217605
*/
// trim from start
inline std::string& LTrim( std::string& s )
{
	s.erase( s.begin(), std::find_if( s.begin(), s.end(),
									  std::not1( std::ptr_fun<int, int>( std::isspace ) ) ) );
	return s;
}

// trim from end
inline std::string& RTrim( std::string& s )
{
	s.erase( std::find_if( s.rbegin(), s.rend(),
						   std::not1( std::ptr_fun<int, int>( std::isspace ) ) ).base(), s.end() );
	return s;
}

// trim from both ends
inline std::string& Trim( std::string& s )
{
	return LTrim( RTrim( s ) );
}

/*
*	End based on code.
*/

/**
*	Checks if a printf operation was successful
*/
inline bool PrintfSuccess( const int iRet, const size_t uiBufferSize )
{
	return iRet >= 0 && static_cast<size_t>( iRet ) < uiBufferSize;
}
}

#endif //ANGELSCRIPT_UTIL_STRINGUTILS_H