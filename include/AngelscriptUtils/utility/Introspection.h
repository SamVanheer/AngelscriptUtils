#pragma once

#include <stdexcept>
#include <string>

#include <angelscript.h>

#include "AngelscriptUtils/util/ContextUtils.h"
#include "AngelscriptUtils/wrapper/CASArguments.h"

namespace asutils
{
/**
*	@brief Iterates over a list of functions
*/
struct FunctionIterator final
{
	const asIScriptModule& module;

	/**
	*	@brief Creates an iterator for the given module's functions
	*/
	FunctionIterator(const asIScriptModule& module)
		: module(module)
	{
	}

	FunctionIterator(const FunctionIterator&) = default;
	FunctionIterator& operator=(const FunctionIterator&) = delete;

	asUINT GetCount() const { return module.GetFunctionCount(); }

	/**
	*	@brief Gets a function by index
	*/
	asIScriptFunction* GetByIndex(const asUINT uiIndex) const
	{
		return module.GetFunctionByIndex(uiIndex);
	}
};

/**
*	@brief Iterates over a list of object methods
*/
struct MethodIterator final
{
	const asITypeInfo& objectType;

	/**
	*	Constructor.
	*	@param objectType Object type info.
	*/
	MethodIterator(const asITypeInfo& objectType)
		: objectType(objectType)
	{
		//Must be an object
		//TODO: asOBJ_VALUE should also count - Solokiller
		if (!(objectType.GetFlags() & asOBJ_REF))
		{
			throw std::invalid_argument("Value types are not supported");
		}
	}

	MethodIterator(const MethodIterator& other) = default;
	MethodIterator& operator=(const MethodIterator&) = delete;

	asUINT GetCount() const { return objectType.GetMethodCount(); }

	/**
	*	@brief Gets a method by index
	*/
	asIScriptFunction* GetByIndex(const asUINT uiIndex) const
	{
		return objectType.GetMethodByIndex(uiIndex);
	}
};

/**
*	@brief Finds a function by name and argument types
*	@param funcIterator Function iterator
*	@param arguments Function arguments
*	@param bExplicitReturnType Whether the return type should be checked. Compared against iReturnTypeId
*	@param iReturnTypeId The return type id to match against if bExplicitReturnType is true
*	@return Function, or null if no function could be found
*/
template<typename FUNCITERATOR>
inline asIScriptFunction* FindFunction(
	asIScriptEngine& engine,
	const FUNCITERATOR& funcIterator,
	const std::string& functionName,
	CASArguments& arguments,
	const bool explicitReturnType = true,
	const int returnTypeId = asTYPEID_VOID)
{
	const auto& argList = arguments.GetArgumentList();

	for (asUINT uiIndex = 0; uiIndex < funcIterator.GetCount(); ++uiIndex)
	{
		asIScriptFunction* const pFunction = funcIterator.GetByIndex(uiIndex);

		//Name must match
		if (pFunction->GetName() != functionName)
		{
			continue;
		}

		if (explicitReturnType && pFunction->GetReturnTypeId() != returnTypeId)
		{
			continue;
		}

		//Must match parameter count
		if (pFunction->GetParamCount() != arguments.GetArgumentCount())
		{
			continue;
		}

		asUINT parameterIndex;

		const asUINT parameterCount = pFunction->GetParamCount();

		//Each parameter must be the correct type
		for (parameterIndex = 0; parameterIndex < parameterCount; ++parameterIndex)
		{
			int typeId;
			asDWORD flags;

			if (pFunction->GetParam(parameterIndex, &typeId, &flags) < 0)
			{
				const auto szCandidateFuncName = asutils::FormatFunctionName(*pFunction);
				as::log->critical("as::FindFunction: Failed to retrieve parameter {} for function {}!", parameterIndex, szCandidateFuncName);
				break;
			}

			//Can be null in case of primitive types or enums
			auto type = engine.GetTypeInfoById(typeId);

			const auto& arg = argList[parameterIndex];

			const ArgType::ArgType argumentType = ctx::ArgumentTypeFromTypeId(typeId, type ? type->GetFlags() : 0);

			if (arg.GetArgumentType() != argumentType)
			{
				//Enum to primitive conversion is handled by the next check
				if (arg.GetArgumentType() != ArgType::ENUM || argumentType != ArgType::PRIMITIVE)
				{
					break;
				}
			}

			//Make sure only to consider the base id + object flag
			if ((arg.GetTypeId() & (asTYPEID_MASK_OBJECT | asTYPEID_MASK_SEQNBR)) != (typeId & (asTYPEID_MASK_OBJECT | asTYPEID_MASK_SEQNBR)))
			{
				ArgumentValue value;

				//Warning: either of these can modify value
				if (ctx::ConvertEnumToPrimitive(arg, typeId, value))
				{
					//TODO: handle conversion during call.
					/*
					//Change the type to match
					if (!arg.Set(typeId, arg.GetArgumentType(), value, false))
					{
						as::log->critical("as::FindFunction: Failed to convert enum value!");
						break;
					}
					*/
				}
				else
				{
					break;
				}
			}
		}

		//Validation passed
		if (parameterIndex == parameterCount)
		{
			return pFunction;
		}
	}

	return nullptr;
}
}
