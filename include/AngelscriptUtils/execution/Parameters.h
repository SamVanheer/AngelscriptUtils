#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <angelscript.h>

#include "AngelscriptUtils/execution/Metadata.h"

#include "AngelscriptUtils/utility/SmartPointers.h"
#include "AngelscriptUtils/utility/TypeInfo.h"
#include "AngelscriptUtils/utility/Variant.h"

namespace asutils
{
/**
*	@brief Sets a primitive type parameter
*/
template<typename PARAM, typename std::enable_if_t<IsPrimitiveType<PARAM>::value, int> = 0>
bool SetPrimitiveParameter(asIScriptContext& context, const asUINT index, const BaseObjectType& type, PARAM& parameter, const int typeId, const asDWORD flags)
{
	if (flags & asTM_INOUTREF)
	{
		if (typeId != type.GetPrimitiveTypeId())
		{
			context.GetEngine()->WriteMessage("asutils::SetPrimitiveParameter", -1, -1, asMSGTYPE_ERROR,
				"Cannot pass primitive values by reference when the types don't match");
			return false;
		}

		return context.SetArgAddress(index, &parameter) >= 0;
	}
	else
	{
#define PRIMITIVE_CASE(primitiveTypeId, function, destinationType)									\
case primitiveTypeId: return context.function(index, static_cast<destinationType>(parameter)) >= 0

		switch (typeId)
		{
			//This happens if the caller passes in a void pointer
			//Execution should never reach this point due to a lack of metadata for void but just in case
		case asTYPEID_VOID:
			context.GetEngine()->WriteMessage("asutils::SetPrimitiveParameter", -1, -1, asMSGTYPE_ERROR, "void parameter types are not supported");
			return false;

		PRIMITIVE_CASE(asTYPEID_BOOL, SetArgByte, bool);

		PRIMITIVE_CASE(asTYPEID_INT8, SetArgByte, std::int8_t);
		PRIMITIVE_CASE(asTYPEID_INT16, SetArgWord, std::int16_t);
		PRIMITIVE_CASE(asTYPEID_INT32, SetArgDWord, std::int32_t);
		PRIMITIVE_CASE(asTYPEID_INT64, SetArgQWord, std::int64_t);

		PRIMITIVE_CASE(asTYPEID_UINT8, SetArgByte, std::uint8_t);
		PRIMITIVE_CASE(asTYPEID_UINT16, SetArgWord, std::uint16_t);
		PRIMITIVE_CASE(asTYPEID_UINT32, SetArgDWord, std::uint32_t);
		PRIMITIVE_CASE(asTYPEID_UINT64, SetArgQWord, std::uint64_t);

		PRIMITIVE_CASE(asTYPEID_FLOAT, SetArgFloat, float);
		PRIMITIVE_CASE(asTYPEID_DOUBLE, SetArgDouble, double);

			//Should never get here
		default:
			//TODO: could get the type info and print the actual type
			context.GetEngine()->WriteMessage("asutils::SetPrimitiveParameter", -1, -1, asMSGTYPE_ERROR,
				"Non-primitive types are not supported by SetPrimitiveParameter");
			return false;
		}

#undef PRIMITIVE_CASE
	}
}

template<typename PARAM, typename std::enable_if_t<std::is_enum_v<PARAM>, int> = 0>
bool SetPrimitiveParameter(asIScriptContext& context, const asUINT index, const BaseObjectType& type, PARAM& parameter, const int typeId, const asDWORD flags)
{
	//Overload for enums passed in to primitive types
	return SetPrimitiveParameter(context, index, type, reinterpret_cast<std::underlying_type_t<PARAM>&>(parameter), typeId, flags);
}

template<typename PARAM, typename std::enable_if_t<!IsPrimitiveType<PARAM>::value && !std::is_enum_v<PARAM>, int> = 0>
bool SetPrimitiveParameter(asIScriptContext& context, const asUINT, const BaseObjectType&, PARAM&, const int, const asDWORD)
{
	//Should never be reached; only required for completeness
	context.GetEngine()->WriteMessage("asutils::SetPrimitiveParameter", -1, -1, asMSGTYPE_ERROR,
		"Non-primitive types are not supported by SetPrimitiveParameter");
	return false;
}

/**
*	@brief Sets an enum parameter
*	If the parameter type is an enum with a 32 bit signed integer underlying type it will be passed directly
*	If the parameter type is an enum with a non-32 bit signed integer type it will be converted to a 32 bit signed integer
*	If the parameter type is an integral 32 bit signed integer type it will be passed directly
*	If the parameter type is an integral non-32 bit signed integer type it will be converted to a 32 bit signed integer
*	Any other types will result in an error
*	Ïf the type has to be converted reference parameters will not be supported
*/
template<typename PARAM, std::enable_if_t<std::is_enum_v<PARAM> && Is32BitEnum<PARAM>::value, int> = 0>
bool SetEnumParameter(asIScriptFunction& function, asIScriptContext& context,
	const asUINT index, const BaseObjectType& type, PARAM&& parameter, const asDWORD flags)
{
	//If it's a 32 bit signed integer we can support reference parameters
	if (flags & asTM_INOUTREF)
	{
		return context.SetArgAddress(index, &parameter) >= 0;
	}
	else
	{
		return context.SetArgDWord(index, parameter) >= 0;
	}
}

template<typename PARAM, std::enable_if_t<std::is_enum_v<PARAM> && !Is32BitEnum<PARAM>::value, int> = 0>
bool SetEnumParameter(asIScriptFunction& function, asIScriptContext& context,
	const asUINT index, const BaseObjectType& type, PARAM&& parameter, const asDWORD flags)
{
	//For non-32 bit signed integers we can't directly pass the address so references are not supported
	if (flags & asTM_INOUTREF)
	{
		context.GetEngine()->WriteMessage("asutils::SetEnumParameter", -1, -1, asMSGTYPE_ERROR,
			"Cannot pass non-32 bit signed integer enum values by reference");
		return false;
	}

	//All enum types are 32 bit signed integers in Angelscript, so convert the value to that and set it
	const auto enumValue = ConvertEnumToInt(parameter);

	return context.SetArgDWord(index, enumValue) >= 0;
}

template<typename PARAM, std::enable_if_t<std::is_integral_v<PARAM> && std::is_same_v<int, PARAM>, int> = 0>
bool SetEnumParameter(asIScriptFunction& function, asIScriptContext& context,
	const asUINT index, const BaseObjectType& type, PARAM&& parameter, const asDWORD flags)
{
	//If it's a 32 bit signed integer we can support reference parameters
	if (flags & asTM_INOUTREF)
	{
		return context.SetArgAddress(index, &parameter) >= 0;
	}
	else
	{
		return context.SetArgDWord(index, parameter) >= 0;
	}
}

template<typename PARAM, std::enable_if_t<std::is_integral_v<PARAM> && !std::is_same_v<int, PARAM>, int> = 0>
bool SetEnumParameter(asIScriptFunction& function, asIScriptContext& context,
	const asUINT index, const BaseObjectType& type, PARAM&& parameter, const asDWORD flags)
{
	//For non-32 bit signed integers we can't directly pass the address so references are not supported
	if (flags & asTM_INOUTREF)
	{
		context.GetEngine()->WriteMessage("asutils::SetEnumParameter", -1, -1, asMSGTYPE_ERROR,
			"Cannot pass non-32 bit signed integer enum values by reference");
		return false;
	}

	const auto enumValue = static_cast<asDWORD>(parameter);

	return context.SetArgDWord(index, enumValue) >= 0;
}

template<typename PARAM, std::enable_if_t<!std::is_enum_v<PARAM> && !std::is_integral_v<PARAM>, int> = 0>
bool SetEnumParameter(asIScriptFunction&, asIScriptContext& context,
	const asUINT, const BaseObjectType&, PARAM&&, const asDWORD)
{
	context.GetEngine()->WriteMessage("asutils::SetEnumParameter", -1, -1, asMSGTYPE_ERROR, "SetEnumParameter called with non-enum type");
	return false;
}

template<typename PARAM>
inline bool SetObjectParameter(asIScriptContext& context, const asUINT index, const BaseObjectType& type, PARAM&& parameter, const int typeId)
{
	auto& engine = *context.GetEngine();

	//Match the source with the destination type
	auto destinationType = engine.GetTypeInfoById(typeId);

	const auto destinationFlags = destinationType->GetFlags();

	if (!AreObjectTypeFlagsSameType(destinationFlags, type.GetFlags()))
	{
		engine.WriteMessage("asutils::SetNativeParameter", -1, -1, asMSGTYPE_ERROR,
			"Object parameters must be the same type (reference or value type)");
		return false;
	}

	//For safety purposes reference types must be passed as a pointer to avoid pointing to a reference on the stack
	if ((destinationFlags & asOBJ_REF) &&
		!std::is_pointer_v<std::remove_reference_t<PARAM>>)
	{
		engine.WriteMessage("asutils::SetNativeParameter", -1, -1, asMSGTYPE_ERROR,
			"Cannot pass reference counted reference by value");
		return false;
	}

	//Don't forward here; we don't want the type to be a reference
	void* object = ConvertObjectToPointer<std::remove_reference_t<PARAM>>(parameter);

	bool performedCast = false;

	//If the types don't match, perform a cast to make them compatible
	if (strcmp(type.GetName(), destinationType->GetName()) ||
		strcmp(type.GetNamespace(), destinationType->GetNamespace()))
	{
		if (destinationFlags & asOBJ_VALUE)
		{
			engine.WriteMessage("asutils::SetNativeParameter", -1, -1, asMSGTYPE_ERROR,
				"Value type conversion is not supported");
			return false;
		}

		const auto sourceTypeName = FormatObjectTypeName(type.GetNamespace(), type.GetName());

		auto sourceType = engine.GetTypeInfoByDecl(sourceTypeName.c_str());

		if (!sourceType)
		{
			engine.WriteMessage("asutils::SetNativeParameter", -1, -1, asMSGTYPE_ERROR,
				(std::string{"The type \""} +sourceTypeName + "\" could not be found for reference casting").c_str());
			return false;
		}

		void* newObject;

		if (engine.RefCastObject(object, sourceType, destinationType, &newObject, false) < 0)
		{
			engine.WriteMessage("asutils::SetNativeParameter", -1, -1, asMSGTYPE_ERROR,
				(std::string{"Could not cast type \""} +sourceTypeName +
					"\" to destination type \"" + FormatObjectTypeName(destinationType->GetNamespace(), destinationType->GetName()) +
					"\"").c_str());
			return false;
		}

		object = newObject;

		performedCast = true;
	}

	const auto success = context.SetArgObject(index, object) >= 0;

	//If we had to cast to pass the object release the reference we acquired as a result
	if (performedCast)
	{
		engine.ReleaseScriptObject(object, destinationType);
	}

	return success;
}

/**
*	@brief Sets a native parameter
*/
template<typename PARAM>
bool SetNativeParameter(asIScriptFunction& function, asIScriptContext& context, const asUINT index, const BaseObjectType& type, PARAM&& parameter)
{
	//TODO: handle pointer-to and reference-to parameters

	int typeId;
	asDWORD flags;

	if (function.GetParam(index, &typeId, &flags) < 0)
	{
		context.GetEngine()->WriteMessage("asutils::SetNativeParameter", -1, -1, asMSGTYPE_ERROR, "Error getting parameter info");
		return false;
	}

	//Check for enums first so integral to enum conversion is performed if necessary
	if (IsEnum(typeId))
	{
		return SetEnumParameter(function, context, index, type, std::forward<PARAM>(parameter), flags);
	}
	//Handle all primitive type parameters as well as enum types passed in to primitive types
	else if (IsPrimitive(typeId) && type.GetPrimitiveTypeId() != asTYPEID_MASK_OBJECT)
	{
		//At this point we're either going to pass by reference or convert and set the argument value,
		//so we no longer need the exact type
		return SetPrimitiveParameter<std::remove_reference_t<PARAM>>(context, index, type, parameter, typeId, flags);
	}
	//Handle any object types
	else if (IsObject(typeId) && (type.GetFlags() & (asOBJ_REF | asOBJ_VALUE)))
	{
		return SetObjectParameter(context, index, type, std::forward<PARAM>(parameter), typeId);
	}
	else
	{
		context.GetEngine()->WriteMessage("asutils::SetNativeParameter", -1, -1, asMSGTYPE_ERROR, "Unknown parameter type");
		return false;
	}
}

/**
*	@brief Sets the parameter for the given function on the given context
*/
template<typename PARAM>
bool SetNativeParameter(asIScriptFunction& function, asIScriptContext& context, const asUINT index, PARAM&& param)
{
	const ObjectType<std::remove_reference_t<PARAM>> type;

	return SetNativeParameter(function, context, index, type, param);
}

/**
*	@brief Sets the parameters for the given function on the given context
*/
template<typename PARAM>
bool SetNativeParameters(asIScriptFunction& function, asIScriptContext& context, const asUINT index, PARAM&& param)
{
	return SetNativeParameter(function, context, index, std::forward<PARAM>(param));
}

/**
*	@copydoc SetParameters(asIScriptFunction&, asIScriptContext&, const asUINT, PARAM&&)
*/
template<typename PARAM, typename... PARAMS>
bool SetNativeParameters(asIScriptFunction& function, asIScriptContext& context, const asUINT index, PARAM&& param, PARAMS&&... parameters)
{
	if (!SetNativeParameter(function, context, index, std::forward<PARAM>(param)))
	{
		return false;
	}

	return SetNativeParameters(function, context, index + 1, std::forward<PARAMS>(parameters)...);
}

/**
*	@copydoc SetParameters(asIScriptFunction&, asIScriptContext&, const asUINT, PARAM&&)
*/
inline bool SetNativeParameters(asIScriptFunction&, asIScriptContext&, const asUINT)
{
	//Nothing. Called for no-parameter function calls
	return true;
}

template<typename T>
inline T CastPrimitiveScriptParameter(const Variant& parameter)
{
#define PRIMITIVE_CASE(typeId, type)								\
case typeId: static_cast<T>(parameter.GetPrimitiveValue<type>())

	switch (parameter.GetTypeId())
	{
		PRIMITIVE_CASE(asTYPEID_BOOL, bool);

		PRIMITIVE_CASE(asTYPEID_INT8, std::int8_t);
		PRIMITIVE_CASE(asTYPEID_INT16, std::int16_t);
		PRIMITIVE_CASE(asTYPEID_INT32, std::int32_t);
		PRIMITIVE_CASE(asTYPEID_INT64, std::int64_t);

		PRIMITIVE_CASE(asTYPEID_UINT8, std::uint8_t);
		PRIMITIVE_CASE(asTYPEID_UINT16, std::uint16_t);
		PRIMITIVE_CASE(asTYPEID_UINT32, std::uint32_t);
		PRIMITIVE_CASE(asTYPEID_UINT64, std::uint64_t);

		PRIMITIVE_CASE(asTYPEID_FLOAT, float);
		PRIMITIVE_CASE(asTYPEID_DOUBLE, double);

	default: throw std::invalid_argument("Must be a primitive type id");
	}

#undef PRIMITIVE_CASE
}

inline bool SetPrimitiveScriptParameter(int typeId, asIScriptContext& context, const asUINT index, const Variant& parameter)
{
#define PRIMITIVE_CASE(primitiveTypeId, function, destinationType)													\
case primitiveTypeId: return context.function(index, CastPrimitiveScriptParameter<destinationType>(parameter)) >= 0

	switch (typeId)
	{
		PRIMITIVE_CASE(asTYPEID_BOOL, SetArgByte, bool);

		PRIMITIVE_CASE(asTYPEID_INT8, SetArgByte, std::int8_t);
		PRIMITIVE_CASE(asTYPEID_INT16, SetArgWord, std::int16_t);
		PRIMITIVE_CASE(asTYPEID_INT32, SetArgDWord, std::int32_t);
		PRIMITIVE_CASE(asTYPEID_INT64, SetArgQWord, std::int64_t);

		PRIMITIVE_CASE(asTYPEID_UINT8, SetArgByte, std::uint8_t);
		PRIMITIVE_CASE(asTYPEID_UINT16, SetArgWord, std::uint16_t);
		PRIMITIVE_CASE(asTYPEID_UINT32, SetArgDWord, std::uint32_t);
		PRIMITIVE_CASE(asTYPEID_UINT64, SetArgQWord, std::uint64_t);

		PRIMITIVE_CASE(asTYPEID_FLOAT, SetArgFloat, float);
		PRIMITIVE_CASE(asTYPEID_DOUBLE, SetArgDouble, double);

	default: return false;
	}

#undef PRIMITIVE_CASE
}

inline bool SetScriptParameter(asIScriptFunction& function, asIScriptContext& context, const asUINT index, const BaseObjectType&, const Variant& parameter)
{
	int typeId;

	asDWORD flags;

	if (function.GetParam(index, &typeId, &flags) < 0)
	{
		context.GetEngine()->WriteMessage("asutils::SetScriptParameter", -1, -1, asMSGTYPE_ERROR, "Error getting parameter info");
		return false;
	}

	if (IsPrimitive(parameter.GetTypeId()))
	{
		return SetPrimitiveScriptParameter(typeId, context, index, parameter);
	}
	else if (IsEnum(parameter.GetTypeId()))
	{
		return context.SetArgDWord(index, parameter.GetPrimitiveValue<int>()) >= 0;
	}
	else if (IsObject(parameter.GetTypeId()))
	{
		return context.SetArgObject(index, parameter.GetObject().Get()) >= 0;
	}
	else
	{
		context.GetEngine()->WriteMessage("asutils::SetParameter", -1, -1, asMSGTYPE_ERROR, "Unknown parameter type");
		return false;
	}
}

inline bool VerifyParameterCount(asIScriptContext& context, asUINT expected, asUINT received)
{
	//Sanity check before doing anything to avoid problems later on
	//TODO: add flag to allow ignoring superfluous parameters
	if (expected != received)
	{
		context.GetEngine()->WriteMessage("asutils::Call", -1, -1, asMSGTYPE_ERROR,
			(std::string{"Cannot call function with incorrect parameter count; Expected "} +
				std::to_string(expected) + ", got " + std::to_string(received)).c_str());
		return false;
	}

	return true;
}

template<typename PARAM>
void ReleaseNativeParameters(PARAM&& parameter)
{
	OptionalReferenceAdapter<std::remove_pointer_t<std::remove_reference_t<PARAM>>>::Release(ConvertObjectToReference(parameter));
}

template<typename PARAM, typename... PARAMS>
void ReleaseNativeParameters(PARAM&& parameter, PARAMS&&... parameters)
{
	ReleaseNativeParameters(std::forward<PARAM>(parameter));

	ReleaseNativeParameters(std::forward<PARAMS>(parameters)...);
}

inline void ReleaseNativeParameters()
{
	//Nothing. Called for no-parameter function calls
}
}
