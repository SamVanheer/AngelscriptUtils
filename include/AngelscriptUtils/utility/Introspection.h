#pragma once

#include <stdexcept>
#include <string>

#include <angelscript.h>

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
}
