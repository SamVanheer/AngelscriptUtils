#pragma once

#include <angelscript.h>

namespace asutils
{
/**
*	@brief Type id bits that indicate that a type id refers to an object of some kind (handle or object reference)
*/
constexpr int OBJECT_TYPEID_BITS = asTYPEID_OBJHANDLE | asTYPEID_HANDLETOCONST | asTYPEID_MASK_OBJECT;
/**
*	@brief Returns whether the given type id is that of void
*	Exists mostly for completeness
*/
inline bool IsVoid(const int typeId)
{
	return asTYPEID_VOID == typeId;
}

/**
*	@brief Returns whether a given type id is that of a primitive type
*/
inline bool IsPrimitive(const int typeId)
{
	return asTYPEID_BOOL <= typeId && typeId <= asTYPEID_DOUBLE;
}

/**
*	@brief Returns whether a given type id is that of an enum type
*/
inline bool IsEnum(int typeId)
{
	//Some kind of object
	if ((typeId & OBJECT_TYPEID_BITS) != 0)
	{
		return false;
	}

	typeId &= asTYPEID_MASK_SEQNBR;

	return typeId > asTYPEID_DOUBLE;
}

/**
*	@brief Returns whether a given type id is that of an integer type
*/
inline bool IsInteger(const int typeId)
{
	return (typeId >= asTYPEID_INT8) && (typeId <= asTYPEID_UINT64);
}

/**
*	@brief Returns whether a given type id is that of a float type
*/
inline bool IsFloat(const int typeId)
{
	return (typeId >= asTYPEID_FLOAT) && (typeId <= asTYPEID_DOUBLE);
}

/**
*	@brief Returns whether a given type id is that of an object type
*/
inline bool IsObject(const int typeId)
{
	return (typeId & asTYPEID_MASK_OBJECT) != 0;
}

/**
*	@param type Object type.
*	@return Whether the given type has a default constructor.
*/
bool HasDefaultConstructor(const asITypeInfo& type);
}
