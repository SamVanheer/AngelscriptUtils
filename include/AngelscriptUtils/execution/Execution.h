#pragma once

#include <angelscript.h>

#include "AngelscriptUtils/execution/Packing.h"
#include "AngelscriptUtils/execution/Parameters.h"

#include "AngelscriptUtils/utility/SmartPointers.h"

namespace asutils
{
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
