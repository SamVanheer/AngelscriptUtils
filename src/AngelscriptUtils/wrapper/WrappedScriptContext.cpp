#include <cassert>

#include "AngelscriptUtils/util/ASUtil.h"
#include "AngelscriptUtils/wrapper/WrappedScriptContext.h"

namespace asutils
{
int LoggingScriptContext::Execute()
{
	const auto result = m_Context.Execute();

	if (result != asEXECUTION_FINISHED)
	{
		const auto szFunctionName = as::FormatFunctionName(*m_Context.GetFunction());

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
			const auto szExceptionFunction = as::FormatFunctionName(*m_Context.GetExceptionFunction());

			int iColumn = 0;
			const char* pszSection = nullptr;

			const int iLineNumber = m_Context.GetExceptionLineNumber(&iColumn, &pszSection);

			if (pszSection == nullptr)
			{
				pszSection = "Unknown section";
			}

			m_Logger->warn(
				"Exception occurred while executing function \"{}\"\nFunction \"{}\" at line {}, column {} in section \"{}\":\n{}",
				szFunctionName, szExceptionFunction, iLineNumber, iColumn, pszSection, m_Context.GetExceptionString()
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

	as::CASCallerInfo info;

	as::GetCallerInfo(info, &m_Context);

	auto currentFunction = m_Context.GetFunction(m_Context.GetCallstackSize() - 1);

	if (currentFunction)
	{
		const auto functionName = as::FormatFunctionName(*currentFunction);

		m_Logger->error("{} while in function \"{}\" at line {}, column {} in section \"{}\"", action, functionName, info.iLine, info.iColumn, info.pszSection);
	}
	else
	{
		m_Logger->error("{} in unknown function", action);
	}
}
}
