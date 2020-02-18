#pragma once

#include <cstdint>
#include <string>
#include <type_traits>

#include <angelscript.h>

#include "AngelscriptUtils/utility/TypeInfo.h"

/**
*	@file
*
*	Defines constants, types, functions and macros for defining and using metadata for C++ types used with Angelscript
*/

namespace asutils
{
/**
*	@brief Provides object type information, including the script type name and type flags
*/
class BaseObjectType
{
protected:
	BaseObjectType(asDWORD flags, int primitiveTypeId = asTYPEID_MASK_OBJECT)
		: m_Flags(flags)
		, m_PrimitiveTypeId(primitiveTypeId)
	{
	}

public:
	virtual ~BaseObjectType() = default;

	BaseObjectType(const BaseObjectType&) = default;
	BaseObjectType& operator=(const BaseObjectType&) = delete;

	virtual const char* GetNamespace() const = 0;

	virtual const char* GetName() const = 0;

	asDWORD GetFlags() const { return m_Flags; }

	/**
	*	@brief If this is a primitive type, returns the type id. Otherwise, returns asTYPEID_MASK_OBJECT to indicate that it is an object type
	*/
	int GetPrimitiveTypeId() const { return m_PrimitiveTypeId; }

private:
	const asDWORD m_Flags;
	const int m_PrimitiveTypeId;
};

/**
*	@brief Base class for native object types
*/
class NativeObjectType : public BaseObjectType
{
protected:
	NativeObjectType(const char* scriptNamespace, const char* scriptName, asDWORD flags, int typeId = asTYPEID_MASK_OBJECT)
		: BaseObjectType(flags, typeId)
		, m_Namespace(scriptNamespace)
		, m_Name(scriptName)
	{
	}

public:
	NativeObjectType(const NativeObjectType&) = default;

	const char* GetNamespace() const override final { return m_Namespace; }

	const char* GetName() const override final { return m_Name; }

private:
	const char* const m_Namespace;
	const char* const m_Name;
};

/**
*	@brief Used to define object type for a particular C++ type
*
*	Use the DEFINE_OBJECT_TYPE* macros to define an instance for a particular type
*/
template<typename T>
class ObjectType : public NativeObjectType
{
public:
	ObjectType(const char* scriptNamespace, const char* scriptName, asDWORD flags, int typeId = asTYPEID_MASK_OBJECT)
		: NativeObjectType(scriptNamespace, scriptName, flags, typeId)
	{
	}

	ObjectType(const ObjectType&) = default;
};

/**
*	@brief Overload for pointer-to types
*/
template<typename T>
class ObjectType<T*> : public NativeObjectType
{
public:
	using UnderlyingType = ObjectType<T>;

	ObjectType()
		: NativeObjectType(UnderlyingType{}.GetNamespace(), UnderlyingType{}.GetName(), UnderlyingType{}.GetFlags(), UnderlyingType{}.GetPrimitiveTypeId())
	{
	}

	ObjectType(const ObjectType&) = default;
};

/**
*	@brief Object type class that represents a script object
*/
class ScriptObjectType final : public BaseObjectType
{
public:
	ScriptObjectType(std::string&& scriptNamespace, std::string&& scriptName, asDWORD flags, int typeId)
		: BaseObjectType(flags, typeId)
		, m_Namespace(std::move(scriptNamespace))
		, m_Name(std::move(scriptName))
	{
	}

	ScriptObjectType(const ScriptObjectType&) = default;

	const char* GetNamespace() const override { return m_Namespace.c_str(); }

	const char* GetName() const override { return m_Name.c_str(); }

private:
	const std::string m_Namespace;
	const std::string m_Name;
};
}

#define __DEFINE_OBJECT_TYPE(cppName, scriptNamespace, scriptName, typeFlags, primitiveTypeId)	\
namespace asutils																				\
{																								\
template<>																						\
class ObjectType<cppName> : public NativeObjectType												\
{																								\
public:																							\
	ObjectType()																				\
		: NativeObjectType(scriptNamespace, scriptName, typeFlags, primitiveTypeId)				\
	{																							\
	}																							\
};																								\
}

/**
*	@brief Defines an object type specialization for the given type
*/
#define DEFINE_OBJECT_TYPE(cppName, scriptNamespace, scriptName, typeFlags)					\
__DEFINE_OBJECT_TYPE(ppName, #scriptNamespace, #scriptName, typeFlags, asTYPEID_MASK_OBJECT)

/**
*	@brief Defines an object type in the global namespace
*	@see DEFINE_OBJECT_TYPE
*/
#define DEFINE_OBJECT_TYPE_SIMPLE(cppName, scriptName, typeFlags)				\
__DEFINE_OBJECT_TYPE(cppName, "", #scriptName, typeFlags, asTYPEID_MASK_OBJECT)

/**
*	@brief Defines an object type for the given primitive type
*/
#define DEFINE_PRIMITIVE_TYPE(cppName, scriptName, primitiveTypeId)	\
__DEFINE_OBJECT_TYPE(cppName, "", #scriptName, 0, primitiveTypeId)

#define DEFINE_ENUM_TYPE(cppName, scriptNamespace, scriptName)												\
__DEFINE_OBJECT_TYPE(cppName, #scriptNamespace, #scriptName, 0, asutils::DeduceEnumTypeId<cppName>::TypeId)

#define DEFINE_ENUM_TYPE_SIMPLE(cppName, scriptName)											\
__DEFINE_OBJECT_TYPE(cppName, "", #scriptName, 0, asutils::DeduceEnumTypeId<cppName>::TypeId)

DEFINE_PRIMITIVE_TYPE(bool, bool, asTYPEID_BOOL)

DEFINE_PRIMITIVE_TYPE(std::int8_t, int8, asTYPEID_INT8)
DEFINE_PRIMITIVE_TYPE(std::int16_t, int16, asTYPEID_INT16)
DEFINE_PRIMITIVE_TYPE(std::int32_t, int32, asTYPEID_INT32)
DEFINE_PRIMITIVE_TYPE(std::int64_t, int64, asTYPEID_INT64)

DEFINE_PRIMITIVE_TYPE(std::uint8_t, uint8, asTYPEID_UINT8)
DEFINE_PRIMITIVE_TYPE(std::uint16_t, uint16, asTYPEID_UINT16)
DEFINE_PRIMITIVE_TYPE(std::uint32_t, uint32, asTYPEID_UINT32)
DEFINE_PRIMITIVE_TYPE(std::uint64_t, uint64, asTYPEID_UINT64)

DEFINE_PRIMITIVE_TYPE(float, float, asTYPEID_FLOAT)
DEFINE_PRIMITIVE_TYPE(double, double, asTYPEID_DOUBLE)

#undef DEFINE_PRIMITIVE_TYPE
