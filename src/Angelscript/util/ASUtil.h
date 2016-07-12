#ifndef UTIL_ASUTIL_H
#define UTIL_ASUTIL_H

#include <angelscript.h>

namespace as
{
inline constexpr bool IsPrimitive( const int iTypeId )
{
	return asTYPEID_BOOL <= iTypeId && iTypeId <= asTYPEID_DOUBLE;
}

inline constexpr bool IsEnum( const int iTypeId )
{
	return ( iTypeId > asTYPEID_DOUBLE && ( iTypeId & asTYPEID_MASK_OBJECT ) == 0 );
}

inline constexpr bool IsInteger( const int iTypeId )
{
	return ( ( iTypeId >= asTYPEID_INT8 ) && ( iTypeId <= asTYPEID_UINT64 ) );
}

inline constexpr bool IsFloat( const int iTypeId )
{
	return ( ( iTypeId >= asTYPEID_FLOAT ) && ( iTypeId <= asTYPEID_DOUBLE ) );
}
}

#endif //UTIL_ASUTIL_H