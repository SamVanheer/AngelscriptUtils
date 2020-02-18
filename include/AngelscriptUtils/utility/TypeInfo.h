#pragma once

#include <cstdint>
#include <type_traits>

#include <angelscript.h>

namespace asutils
{
/**
*	@brief Type id bits that indicate that a type id refers to an object of some kind (handle or object reference)
*/
constexpr int OBJECT_TYPEID_BITS = asTYPEID_OBJHANDLE | asTYPEID_HANDLETOCONST | asTYPEID_MASK_OBJECT;

/**
*	@brief If T is a primitive type, provides a member const value equal true. For any other type, value is false
*/
template<typename T>
struct IsPrimitiveType : std::bool_constant<std::is_same<bool, T>::value || std::is_arithmetic<T>::value>
{
};

/**
*	@brief Maps a primitive type to an Angelscript type id
*/
template<typename T>
struct PrimitiveTypeToId
{
};

#define DEFINE_PRIMITIVE_TYPE(type, typeId)		\
template<>										\
struct PrimitiveTypeToId<type>					\
{												\
	static constexpr int TypeId = typeId;		\
}

DEFINE_PRIMITIVE_TYPE(bool, asTYPEID_BOOL);

DEFINE_PRIMITIVE_TYPE(std::int8_t, asTYPEID_INT8);
DEFINE_PRIMITIVE_TYPE(std::int16_t, asTYPEID_INT16);
DEFINE_PRIMITIVE_TYPE(std::int32_t, asTYPEID_INT32);
DEFINE_PRIMITIVE_TYPE(std::int64_t, asTYPEID_INT64);

DEFINE_PRIMITIVE_TYPE(std::uint8_t, asTYPEID_UINT8);
DEFINE_PRIMITIVE_TYPE(std::uint16_t, asTYPEID_UINT16);
DEFINE_PRIMITIVE_TYPE(std::uint32_t, asTYPEID_UINT32);
DEFINE_PRIMITIVE_TYPE(std::uint64_t, asTYPEID_UINT64);

DEFINE_PRIMITIVE_TYPE(float, asTYPEID_FLOAT);
DEFINE_PRIMITIVE_TYPE(double, asTYPEID_DOUBLE);

#undef DEFINE_PRIMITIVE_TYPE

/**
*	@brief Deduces the type id of the given enum type
*/
template<typename T>
class DeduceEnumTypeId
{
public:
	static const int TypeId = PrimitiveTypeToId<std::underlying_type_t<T>>::TypeId;
};

/**
*	@brief Tests if T is an enumeration with a 32 bit signed integer underlying type
*/
template<typename T, bool = std::is_enum<T>::value>
class Is32BitEnum
{
public:
	static const bool value = false;
};

template<typename T>
class Is32BitEnum<T, true>
{
public:
	static const bool value = std::is_same<int, std::underlying_type_t<T>>::value;
};

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

/**
*	@brief If type T is an enum, converts value to a 32 bit integer for use as an enum parameter
*	Otherwise returns 0
*/
template<typename T, std::enable_if_t<std::is_enum<T>::value, int> = 0>
asDWORD ConvertEnumToInt(const T value)
{
	return static_cast<asDWORD>(value);
}

template<typename T, std::enable_if_t<!std::is_enum<T>::value, int> = 0>
asDWORD ConvertEnumToInt(const T value)
{
	return 0;
}

/**
*	@brief If type T is not a pointer type, returns a pointer to value
*	Otherwise returns value
*/
template<typename T, std::enable_if_t<std::is_pointer<T>::value, int> = 0>
typename std::remove_pointer_t<T>* ConvertObjectToPointer(T& object)
{
	return object;
}

template<typename T, std::enable_if_t<!std::is_pointer<T>::value, int> = 0>
typename std::remove_pointer_t<T>* ConvertObjectToPointer(T& object)
{
	return &object;
}

/**
*	@brief If type T is a pointer type, returns a reference to value
*	Otherwise returns value
*/
template<typename T, std::enable_if_t<std::is_pointer<T>::value, int> = 0>
typename std::remove_pointer_t<T>& ConvertObjectToReference(T object)
{
	return *object;
}

template<typename T, std::enable_if_t<!std::is_pointer<T>::value && std::is_reference<T>::value, int> = 0>
typename T ConvertObjectToReference(T object)
{
	return object;
}

template<typename T, std::enable_if_t<!std::is_pointer<T>::value && !std::is_reference<T>::value, int> = 0>
typename T& ConvertObjectToReference(T& object)
{
	return object;
}

/**
*	@brief Returns whether both object type flags refer to the same type of object, i.e. are both reference or value types
*/
inline bool AreObjectTypeFlagsSameType(asDWORD left, asDWORD right)
{
	left &= (asOBJ_REF | asOBJ_VALUE);
	right &= (asOBJ_REF | asOBJ_VALUE);

	return left == right;
}
}
