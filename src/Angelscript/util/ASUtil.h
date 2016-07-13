#ifndef UTIL_ASUTIL_H
#define UTIL_ASUTIL_H

#include <angelscript.h>

/**
*	@defgroup ASUtil Angelscript Utility Code
*
*	@{
*/

namespace as
{
/**
*	Returns whether the given type id is that of void.
*	Exists mostly for completeness.
*	@param iTypeId Type Id.
*	@return true if it is void, false otherwise.
*/
inline constexpr bool IsVoid( const int iTypeId )
{
	return asTYPEID_VOID == iTypeId;
}

/**
*	Returns whether a given type id is that of a primitive type.
*	@param iTypeId Type Id.
*	@return true if it is a primitive type, false otherwise.
*/
inline constexpr bool IsPrimitive( const int iTypeId )
{
	return asTYPEID_BOOL <= iTypeId && iTypeId <= asTYPEID_DOUBLE;
}

/**
*	Returns whether a given type id is that of an enum type.
*	@param iTypeId Type Id.
*	@return true if it is an enum type, false otherwise.
*/
inline constexpr bool IsEnum( const int iTypeId )
{
	return ( iTypeId > asTYPEID_DOUBLE && ( iTypeId & asTYPEID_MASK_OBJECT ) == 0 );
}

/**
*	Returns whether a given type id is that of an integer type.
*	@param iTypeId Type Id.
*	@return true if it is an integer type, false otherwise.
*/
inline constexpr bool IsInteger( const int iTypeId )
{
	return ( ( iTypeId >= asTYPEID_INT8 ) && ( iTypeId <= asTYPEID_UINT64 ) );
}

/**
*	Returns whether a given type id is that of a float type.
*	@param iTypeId Type Id.
*	@return true if it is a float type, false otherwise.
*/
inline constexpr bool IsFloat( const int iTypeId )
{
	return ( ( iTypeId >= asTYPEID_FLOAT ) && ( iTypeId <= asTYPEID_DOUBLE ) );
}

/**
*	Returns whether a given type id is that of a primitive type.
*	@param iTypeId Type Id.
*	@return true if it is a primitive type, false otherwise.
*/
inline constexpr bool IsObject( const int iTypeId )
{
	return ( iTypeId & asTYPEID_MASK_OBJECT ) != 0;
}

/**
*	Releases a vararg argument.
*	@param engine Script engine.
*	@param pObject Object pointer.
*	@param iTypeId Type Id.
*/
void ReleaseVarArg( asIScriptEngine& engine, void* pObject, const int iTypeId );
}

/** @} */

#endif //UTIL_ASUTIL_H