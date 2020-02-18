#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <angelscript.h>

#include "AngelscriptUtils/execution/Metadata.h"
#include "AngelscriptUtils/execution/Parameters.h"

#include "AngelscriptUtils/utility/SmartPointers.h"
#include "AngelscriptUtils/utility/TypeInfo.h"
#include "AngelscriptUtils/utility/TypeStringUtils.h"
#include "AngelscriptUtils/utility/Variant.h"

/**
*	@file
*
*	Types and functions used to pack parameters for delayed execution
*/

namespace asutils
{
class ScriptParameterErrorException : public std::runtime_error
{
public:
	using std::runtime_error::runtime_error;
};

/**
*	@brief Encapsulates a parameter
*/
class Parameter
{
protected:
	Parameter()
	{
	}

public:
	virtual ~Parameter() = default;

	Parameter(const Parameter&) = delete;
	Parameter& operator=(const Parameter&) = delete;

	virtual bool SetParameter(asIScriptFunction& function, asIScriptContext& context, const asUINT index) = 0;
};

class NativeParameter : public Parameter
{
public:
	/**
	*	@brief Gets the underlying value of this parameter
	*	This method assumes you know the underlying type
	*/
	template<typename T>
	const T& GetValue() const
	{
		return static_cast<const TypedParameter<T>*>(this)->GetValue();
	}

	/**
	*	@copydoc GetValue() const
	*/
	template<typename T>
	T& GetValue()
	{
		return static_cast<TypedParameter<T>*>(this)->GetValue();
	}
};

/**
*	@brief Stores a copy of the given parameter value
*/
template<typename T>
class TypedParameter final : public NativeParameter
{
public:
	TypedParameter(const ObjectType<T>& type, T&& parameter)
		: m_Type(type)
		, m_Parameter(parameter)
	{
	}

	~TypedParameter()
	{
		//Release reference held by this parameter
		OptionalReferenceAdapter<std::remove_pointer_t<std::remove_reference_t<T>>>::Release(ConvertObjectToReference(m_Parameter));
	}

	/**
	*	@brief Gets the underlying value of this parameter
	*/
	const T& GetValue() const { return m_Parameter; }

	/**
	*	@copydoc GetValue() const
	*/
	T& GetValue() { return m_Parameter; }

	bool SetParameter(asIScriptFunction& function, asIScriptContext& context, const asUINT index) override
	{
		return SetNativeParameter(function, context, index, m_Type, m_Parameter);
	}

private:
	const ObjectType<T> m_Type;
	T m_Parameter;
};

class ScriptParameter final : public Parameter
{
public:
	ScriptParameter(const ScriptObjectType& type, Variant&& parameter)
		: m_Type(type)
		, m_Parameter(std::move(parameter))
	{
	}

	~ScriptParameter() = default;

	bool SetParameter(asIScriptFunction& function, asIScriptContext& context, const asUINT index) override
	{
		return SetScriptParameter(function, context, index, m_Type, m_Parameter);
	}

private:
	const ScriptObjectType m_Type;
	Variant m_Parameter;
};

using NativeParameters = std::vector<std::unique_ptr<NativeParameter>>;
using ScriptParameters = std::vector<std::unique_ptr<ScriptParameter>>;

inline void AddParametersToList(NativeParameters&)
{
	//End of list or no parameters provided; done
}

template<typename PARAM, typename... PARAMS>
void AddParametersToList(NativeParameters& list, PARAM&& parameter, PARAMS&&... parameters)
{
	//Use the underlying type for type info
	list.emplace_back(std::make_unique<TypedParameter<PARAM>>(ObjectType<PARAM>{}, std::forward<PARAM>(parameter)));

	AddParametersToList(list, std::forward<PARAMS>(parameters)...);
}

template<typename... PARAMS>
NativeParameters CreateNativeParameterList(PARAMS&&... parameters)
{
	NativeParameters list;

	list.reserve(sizeof...(parameters));

	AddParametersToList(list, std::forward<PARAMS>(parameters)...);

	return list;
}

/**
*	@brief Creates a @see Parameter that contains a copy of the given parameter value
*/
inline std::unique_ptr<ScriptParameter> CreateScriptParameter(asIScriptEngine& engine, const int typeId, void* parameter)
{
	if (IsPrimitive(typeId))
	{
		return std::make_unique<ScriptParameter>(
			ScriptObjectType{"", PrimitiveTypeIdToString(typeId), 0, typeId},
			Variant{engine, typeId, parameter});
	}
	else if (IsEnum(typeId))
	{
		auto type = engine.GetTypeInfoById(typeId);

		return std::make_unique<ScriptParameter>(
			ScriptObjectType{type->GetNamespace(), type->GetName(), type->GetFlags(), typeId},
			Variant{engine, typeId, parameter});
	}
	else if (IsObject(typeId))
	{
		auto type = engine.GetTypeInfoById(typeId);

		ObjectPointer object{parameter, ReferencePointer<asITypeInfo>{type}};

		return std::make_unique<ScriptParameter>(
			ScriptObjectType{type->GetNamespace(), type->GetName(), type->GetFlags(), typeId},
			Variant{std::move(object)});
	}
	else
	{
		throw ScriptParameterErrorException("Unknown type");
	}
}

/**
*	@brief Creates a parameter list from a generic function call
*	@param parameters Generic call instance containing parameter info
*	@param startIndex The index of the first parameter to create a list from
*	@exception throw ScriptParameterErrorException If a parameter has an unknown type,
*		if startIndex is out of range or 
*		if there was an error getting a parameter type id
*/
inline ScriptParameters CreateGenericParameterList(asIScriptGeneric& parameters, const asUINT startIndex = 0)
{
	const asUINT parameterCount = parameters.GetArgCount();

	if (startIndex >= parameterCount)
	{
		throw ScriptParameterErrorException("Starting index out of range");
	}

	ScriptParameters list;

	list.reserve(parameterCount - startIndex);

	auto& engine = *parameters.GetEngine();

	for (asUINT index = startIndex; index < parameterCount; ++index)
	{
		asDWORD flags;

		const auto typeId = parameters.GetArgTypeId(index, &flags);

		if (typeId < 0)
		{
			throw ScriptParameterErrorException("Error getting parameter type id");
		}

		list.emplace_back(CreateScriptParameter(engine, typeId, parameters.GetArgAddress(index)));
	}

	return list;
}

template<typename T>
inline bool SetScriptParameters(asIScriptFunction& function, const T& parameters, asIScriptContext& context)
{
	asUINT index = 0;

	for (const auto& param : parameters)
	{
		if (!param->SetParameter(function, context, index++))
		{
			return false;
		}
	}

	return true;
}
}
