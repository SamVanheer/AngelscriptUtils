#include <cassert>

#include "AngelscriptUtils/utility/ContextUtils.h"
#include "AngelscriptUtils/utility/TypeStringUtils.h"
#include "AngelscriptUtils/wrapper/WrappedScriptContext.h"

namespace asutils
{
int LoggingScriptContext::Execute()
{
	const auto result = m_Context.Execute();

	if (result != asEXECUTION_FINISHED)
	{
		const auto szFunctionName = FormatFunctionName(*m_Context.GetFunction());

		switch (result)
		{
		case asEXECUTION_SUSPENDED:
		{
			//Suspended execution is an error if the user wants it to be.
			if (m_SuspendIsError)
			{
				m_Logger->error("Script execution unexpectedly suspended while executing function \"{}\"", szFunctionName);

				LogCurrentFunction("Suspended");
			}

			break;
		}

		case asCONTEXT_NOT_PREPARED:
		{
			m_Logger->error("Context not prepared to execute function \"{}\"", szFunctionName);
			break;
		}

		case asEXECUTION_ABORTED:
		{
			m_Logger->error("Script execution aborted while executing function \"{}\"", szFunctionName);

			LogCurrentFunction("Aborted");

			break;
		}

		case asEXECUTION_EXCEPTION:
		{
			const auto szExceptionFunction = FormatFunctionName(*m_Context.GetExceptionFunction());

			asutils::LocationInfo info;

			asutils::GetExceptionInfo(m_Context, info);

			m_Logger->warn(
				"Exception occurred while executing function \"{}\"\nFunction \"{}\" at line {}, column {} in section \"{}\":\n{}",
				szFunctionName, szExceptionFunction, info.line, info.column, info.section, m_Context.GetExceptionString()
			);

			break;
		}

		//Unexpected error states.
		case asEXECUTION_FINISHED:
		case asEXECUTION_PREPARED:
		case asEXECUTION_UNINITIALIZED:
		case asEXECUTION_ACTIVE:
		case asEXECUTION_ERROR:
		{
			m_Logger->error("Unexpected context state \"{}\" encountered while executing function \"{}\"", result, szFunctionName);
			break;
		}

		//Unknown error.
		default:
		case asERROR:
		{
			m_Logger->error("Unknown error \"{}\" occurred while executing function \"{}\"\n", result, szFunctionName);
			break;
		}
		}
	}

	return result;
}

void LoggingScriptContext::LogCurrentFunction(const char* const action)
{
	assert(action);
	assert(*action);

	asutils::LocationInfo info;

	asutils::GetCallerInfo(m_Context, info);

	auto currentFunction = m_Context.GetFunction(m_Context.GetCallstackSize() - 1);

	if (currentFunction)
	{
		const auto functionName = FormatFunctionName(*currentFunction);

		m_Logger->error("{} while in function \"{}\" at line {}, column {} in section \"{}\"", action, functionName, info.line, info.column, info.section);
	}
	else
	{
		m_Logger->error("{} in unknown function", action);
	}
}
}
