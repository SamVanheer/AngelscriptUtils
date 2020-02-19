#pragma once

#include <stdexcept>
#include <string>

#include <angelscript.h>

namespace asutils
{
//TODO: rewrite these to use begin() and end() for iteration

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
	*	Creates an iterator for the given object type's methods
	*/
	MethodIterator(const asITypeInfo& objectType)
		: objectType(objectType)
	{
		if (!(objectType.GetFlags() & (asOBJ_REF | asOBJ_VALUE)))
		{
			throw std::invalid_argument("MethodIterator type must be an object type");
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
