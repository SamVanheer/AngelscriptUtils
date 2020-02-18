#pragma once

#include <optional>
#include <stdexcept>

#include <angelscript.h>

#include "AngelscriptUtils/execution/Metadata.h"
#include "AngelscriptUtils/execution/Packing.h"
#include "AngelscriptUtils/execution/Parameters.h"

#include "AngelscriptUtils/utility/SmartPointers.h"
#include "AngelscriptUtils/utility/TypeInfo.h"
#include "AngelscriptUtils/utility/TypeStringUtils.h"

namespace asutils
{
/**
*	@brief Base class for callable functions
*/
class CallableFunction
{
protected:
	CallableFunction(asIScriptFunction& function, asIScriptContext& context)
		: m_Function(function)
		, m_Context(context)
	{
	}

public:
	CallableFunction(CallableFunction&&) = default;
	CallableFunction& operator=(CallableFunction&&) = delete;

	/**
	*	@brief Gets the return primitive from the last call, if any
	*/
	template<typename T, std::enable_if_t<IsPrimitiveType<T>::value, int> = 0>
	std::optional<T> GetPrimitiveReturnValue()
	{
		asDWORD flags;
		const auto returnTypeId = m_Function.GetReturnTypeId(&flags);

		if (returnTypeId == asTYPEID_VOID)
		{
			m_Context.GetEngine()->WriteMessage("CallableFunction::GetReturnValue", -1, -1, asMSGTYPE_ERROR,
				"Application error: Cannot return value of type void");
			return {};
		}

		if (flags & asTM_INOUTREF)
		{
			m_Context.GetEngine()->WriteMessage("CallableFunction::GetReturnValue", -1, -1, asMSGTYPE_ERROR,
				"Return by reference is not supported for primitive types and enums");
			return {};
		}

		if (IsPrimitive(returnTypeId))
		{
			return CastReturnValueToType<T>(returnTypeId, m_Context);
		}

		return {};
	}

	/**
	*	@brief Gets the return enum from the last call, if any
	*/
	template<typename T, std::enable_if_t<std::is_enum<T>::value, int> = 0>
	std::optional<T> GetEnumReturnValue()
	{
		asDWORD flags;
		const auto returnTypeId = m_Function.GetReturnTypeId(&flags);

		if (returnTypeId == asTYPEID_VOID)
		{
			m_Context.GetEngine()->WriteMessage("CallableFunction::GetReturnValue", -1, -1, asMSGTYPE_ERROR,
				"Application error: Cannot return value of type void");
			return {};
		}

		if (flags & asTM_INOUTREF)
		{
			m_Context.GetEngine()->WriteMessage("CallableFunction::GetReturnValue", -1, -1, asMSGTYPE_ERROR,
				"Return by reference is not supported for primitive types and enums");
			return {};
		}

		if (IsEnum(returnTypeId))
		{
			return CastReturnValueToType<T>(asTYPEID_INT32, m_Context);
		}

		return {};
	}

	/**
	*	@brief Gets the return object from the last call, if any
	*/
	ObjectPointer GetObjectReturnValue()
	{
		asDWORD flags;
		const auto returnTypeId = m_Function.GetReturnTypeId(&flags);

		if (returnTypeId == asTYPEID_VOID)
		{
			m_Context.GetEngine()->WriteMessage("CallableFunction::GetReturnValue", -1, -1, asMSGTYPE_ERROR,
				"Application error: Cannot return value of type void");
			return {};
		}

		if (IsObject(returnTypeId))
		{
			auto type = m_Context.GetEngine()->GetTypeInfoById(returnTypeId);

			return {m_Context.GetReturnObject(), ReferencePointer<asITypeInfo>(type)};
		}

		return {};
	}

protected:
	asIScriptFunction& m_Function;
	asIScriptContext& m_Context;
};

/**
*	@brief Provides a RAII wrapper around function execution
*	Can be used to examine the result of an execution
*	Should always be used as a temporary variable on the stack
*	Relies on the user to maintain a reference to the function and context
*/
class CallableGlobalFunction final : public CallableFunction
{
protected:
	friend class FunctionExecutor;

	CallableGlobalFunction(asIScriptFunction& function, asIScriptContext& context)
		: CallableFunction(function, context)
	{
	}

public:
	CallableGlobalFunction(CallableGlobalFunction&&) = default;
	CallableGlobalFunction& operator=(CallableGlobalFunction&&) = delete;

	template<typename... PARAMS>
	bool NativeCall(PARAMS&&... parameters)
	{
		bool success = false;

		if (VerifyParameterCount(m_Context, m_Function.GetParamCount(), sizeof...(parameters)))
		{
			auto result = m_Context.Prepare(&m_Function);

			if (result >= 0)
			{
				if (SetNativeParameters(m_Function, m_Context, 0, std::forward<PARAMS>(parameters)...))
				{
					result = m_Context.Execute();
				}
				else
				{
					result = asERROR;
				}
			}

			success = result >= 0;
		}

		//Release references to caller parameters
		ReleaseNativeParameters(std::forward<PARAMS>(parameters)...);

		return success;
	}

	template<typename T>
	inline bool PackedCall(const T& parameters)
	{
		bool success = false;

		if (VerifyParameterCount(m_Context, m_Function.GetParamCount(), parameters.size()))
		{
			auto result = m_Context.Prepare(&m_Function);

			if (result >= 0)
			{
				if (SetScriptParameters(m_Function, parameters, m_Context))
				{
					result = m_Context.Execute();
				}
				else
				{
					result = asERROR;
				}
			}

			success = result >= 0;
		}

		return success;
	}
};

/**
*	@brief Provides a RAII wrapper around function execution
*	Can be used to examine the result of an execution
*	Should always be used as a temporary variable on the stack
*	Relies on the user to maintain a reference to the function and context
*/
class CallableMemberFunction final : public CallableFunction
{
protected:
	friend class FunctionExecutor;

	CallableMemberFunction(asIScriptFunction& function, asIScriptContext& context)
		: CallableFunction(function, context)
	{
	}

public:
	CallableMemberFunction(CallableMemberFunction&&) = default;
	CallableMemberFunction& operator=(CallableMemberFunction&&) = delete;

	template<typename... PARAMS>
	bool NativeCall(void* instance, PARAMS&&... parameters)
	{
		bool success = false;

		if (instance)
		{
			if (VerifyParameterCount(m_Context, m_Function.GetParamCount(), sizeof...(parameters)))
			{
				auto result = m_Context.Prepare(&m_Function);

				if (result >= 0)
				{
					result = m_Context.SetObject(instance);
				}

				if (result >= 0)
				{
					if (SetNativeParameters(m_Function, m_Context, 0, std::forward<PARAMS>(parameters)...))
					{
						result = m_Context.Execute();
					}
					else
					{
						result = asERROR;
					}
				}

				success = result >= 0;
			}
		}
		else
		{
			m_Context.GetEngine()->WriteMessage("CallableMemberFunction::NativeCall", -1, -1, asMSGTYPE_ERROR,
				"Application error: Object instance is null");
		}

		//Release references to caller parameters
		ReleaseNativeParameters(std::forward<PARAMS>(parameters)...);

		return success;
	}

	template<typename T>
	inline bool PackedCall(void* instance, const T& parameters)
	{
		bool success = false;

		if (instance)
		{
			if (VerifyParameterCount(m_Context, m_Function.GetParamCount(), parameters.size()))
			{
				auto result = m_Context.Prepare(&m_Function);

				if (result >= 0)
				{
					result = m_Context.SetObject(instance);
				}

				if (result >= 0)
				{
					if (SetScriptParameters(m_Function, parameters, m_Context))
					{
						result = m_Context.Execute();
					}
					else
					{
						result = asERROR;
					}
				}

				success = result >= 0;
			}
		}
		else
		{
			m_Context.GetEngine()->WriteMessage("CallableMemberFunction::PackedCall", -1, -1, asMSGTYPE_ERROR,
				"Application error: Object instance is null");
		}

		return success;
	}
};

/**
*	@brief Provides a RAII wrapper around function execution
*	Can be used to examine the result of an execution
*	Should always be used as a temporary variable on the stack
*	Relies on the user to maintain a reference to the function and context
*/
class FunctionExecutor final
{
public:
	FunctionExecutor(asIScriptContext& context)
		: m_Context(context)
	{
	}

	~FunctionExecutor()
	{
		//Clean up any calls that were made
		m_Context.Unprepare();
	}

	FunctionExecutor(const FunctionExecutor&) = delete;
	FunctionExecutor& operator=(const FunctionExecutor&) = delete;

	CallableGlobalFunction Global(asIScriptFunction& function)
	{
		if (function.GetObjectType())
		{
			throw std::invalid_argument("Cannot call member function as a global function");
		}

		return CallableGlobalFunction(function, m_Context);
	}

	CallableMemberFunction Member(asIScriptFunction& function)
	{
		if (!function.GetObjectType())
		{
			throw std::invalid_argument("Cannot call global function as a member function");
		}

		return CallableMemberFunction(function, m_Context);
	}

private:
	asIScriptContext& m_Context;
};

/**
*	@brief Calls the given function with the given context, passing the given parameters
*	@return Whether execution succeeded without errors
*/
template<typename... PARAMS>
bool Call(asIScriptFunction& function, asIScriptContext& context, PARAMS&&... parameters)
{
	bool success = false;

	if (VerifyParameterCount(context, function.GetParamCount(), sizeof...(parameters)))
	{
		auto result = context.Prepare(&function);

		if (result >= 0)
		{
			if (SetNativeParameters(function, context, 0, std::forward<PARAMS>(parameters)...))
			{
				result = context.Execute();
			}
			else
			{
				result = asERROR;
			}
		}

		//Always unprepare to clean up state
		const auto unprepareResult = context.Unprepare();

		success = result >= 0 && unprepareResult >= 0;
	}

	//Release references to caller parameters
	ReleaseNativeParameters(std::forward<PARAMS>(parameters)...);

	return success;
}

/**
*	@brief Calls the given function with the given context, passing the given parameters
*	@return Whether execution succeeded without errors
*/
template<typename T>
inline bool Call(asIScriptFunction& function, const T& parameters, asIScriptContext& context)
{
	bool success = false;

	if (VerifyParameterCount(context, function.GetParamCount(), parameters.size()))
	{
		auto result = context.Prepare(&function);

		if (result >= 0)
		{
			if (SetScriptParameters(function, parameters, context))
			{
				result = context.Execute();
			}
			else
			{
				result = asERROR;
			}
		}

		//Always unprepare to clean up state
		const auto unprepareResult = context.Unprepare();

		success = result >= 0 && unprepareResult >= 0;
	}

	return success;
}
}
