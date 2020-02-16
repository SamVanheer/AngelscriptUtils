#pragma once

#include <angelscript.h>

namespace asutils
{
/**
*	@brief Returns whether the given type id is that of void
*	Exists mostly for completeness
*/
inline bool IsVoid(const int iTypeId)
{
	return asTYPEID_VOID == iTypeId;
}

/**
*	@brief Returns whether a given type id is that of a primitive type
*/
inline bool IsPrimitive(const int iTypeId)
{
	return asTYPEID_BOOL <= iTypeId && iTypeId <= asTYPEID_DOUBLE;
}

/**
*	@brief Returns whether a given type id is that of an enum type
*/
inline bool IsEnum(const int iTypeId)
{
	return iTypeId > asTYPEID_DOUBLE && (iTypeId & asTYPEID_MASK_OBJECT) == 0;
}

/**
*	@brief Returns whether a given type id is that of an integer type
*/
inline bool IsInteger(const int iTypeId)
{
	return (iTypeId >= asTYPEID_INT8) && (iTypeId <= asTYPEID_UINT64);
}

/**
*	@brief Returns whether a given type id is that of a float type
*/
inline bool IsFloat(const int iTypeId)
{
	return (iTypeId >= asTYPEID_FLOAT) && (iTypeId <= asTYPEID_DOUBLE);
}

/**
*	@brief Returns whether a given type id is that of an object type
*/
inline bool IsObject(const int iTypeId)
{
	return (iTypeId & asTYPEID_MASK_OBJECT) != 0;
}

/**
*	@param type Object type.
*	@return Whether the given type has a default constructor.
*/
bool HasDefaultConstructor(const asITypeInfo& type);
}
