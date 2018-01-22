#include "util/ASUtil.h"

#include "CASLoggingContextResultHandler.h"

CASLoggingContextResultHandler::CASLoggingContextResultHandler( const Flags_t flags )
{
	SetFlags( flags );
}

void CASLoggingContextResultHandler::ProcessPrepareResult( asIScriptFunction&, asIScriptContext&, int iResult )
{
	if( iResult < 0 )
	{
		switch( iResult )
		{
		case asCONTEXT_ACTIVE:
			{
				as::Critical( "Tried to prepare context that is currently executing or suspended\n" );
				break;
			}

			//This can't happen without throwing an exception when passing the function into as::Call since it dereferences the pointer.
		case asNO_FUNCTION:
			{
				as::Critical( "Tried to prepare null function\n" );
				break;
			}

		case asINVALID_ARG:
			{
				as::Critical( "Tried to prepare context with function from different engine\n" );
				break;
			}

		case asOUT_OF_MEMORY:
			{
				as::Critical( "Ran out of memory while preparing context for execution\n" );
				break;
			}

		default:
			{
				as::Critical( "Unknown error \"%d\" occurred while preparing context for execution\n", iResult );
				break;
			}
		}
	}
}

void CASLoggingContextResultHandler::ProcessExecuteResult( asIScriptFunction& function, asIScriptContext& context, int iResult )
{
	if( iResult != asEXECUTION_FINISHED )
	{
		const auto szFunctionName = as::FormatFunctionName( function );

		switch( iResult )
		{
		case asEXECUTION_SUSPENDED:
			{
				//Suspended execution is an error if the user wants it to be.
				if( m_Flags & Flag::SUSPEND_IS_ERROR )
				{
					as::Critical( "Script execution unexpectedly suspended while executing function \"%s\"\n", szFunctionName.c_str() );

					LogCurrentFunction( context, "Suspended" );
				}

				break;
			}

		case asCONTEXT_NOT_PREPARED:
			{
				as::Critical( "Context not prepared to execute function \"%s\"\n", szFunctionName.c_str() );
				break;
			}

		case asEXECUTION_ABORTED:
			{
				as::Critical( "Script execution aborted while executing function \"%s\"\n", szFunctionName.c_str() );

				LogCurrentFunction( context, "Aborted" );

				break;
			}

		case asEXECUTION_EXCEPTION:
			{
				as::Critical( "Exception occurred while executing function \"%s\"\n", szFunctionName.c_str() );

				const auto szExceptionFunction = as::FormatFunctionName( *context.GetExceptionFunction() );

				int iColumn = 0;
				const char* pszSection = nullptr;

				const int iLineNumber = context.GetExceptionLineNumber( &iColumn, &pszSection );

				as::Critical( "Function \"%s\" at line %d, column %d in section \"%s\":\n%s\n", szExceptionFunction.c_str(), iLineNumber, iColumn, pszSection, context.GetExceptionString() );

				break;
			}

			//Unexpected error states.
		case asEXECUTION_FINISHED:
		case asEXECUTION_PREPARED:
		case asEXECUTION_UNINITIALIZED:
		case asEXECUTION_ACTIVE:
		case asEXECUTION_ERROR:
			{
				as::Critical( "Unexpected context state \"%d\" encountered while executing function \"%s\"\n", iResult, szFunctionName.c_str() );
				break;
			}

			//Unknown error.
		default:
		case asERROR:
			{
				as::Critical( "Unknown error \"%d\" occurred while executing function \"%s\"\n", iResult, szFunctionName.c_str() );
				break;
			}
		}
	}
}

void CASLoggingContextResultHandler::ProcessUnprepareResult( asIScriptContext&, int iResult )
{
	if( iResult < 0 )
	{
		switch( iResult )
		{
			//This can happen if a longjmp occurs while executing a script. The context state is corrupted after such a call.
		case asCONTEXT_ACTIVE:
			{
				as::Critical( "Tried to unprepare context that is still active\n" );
				break;
			}

		default:
			{
				as::Critical( "Unknown error \"%d\" occurred while unpreparing\n", iResult );
				break;
			}
		}
	}
}

void CASLoggingContextResultHandler::LogCurrentFunction( asIScriptContext& context, const char* const pszAction )
{
	assert( pszAction );
	assert( *pszAction );

	as::CASCallerInfo info;

	as::GetCallerInfo( info, &context );

	auto pCurrentFunc = context.GetFunction( context.GetCallstackSize() - 1 );

	if( pCurrentFunc )
	{
		const auto szFunctionName = as::FormatFunctionName( *pCurrentFunc );

		as::Critical( "%s while in function \"%s\" at line %d, column %d in section \"%s\"\n", pszAction, szFunctionName.c_str(), info.iLine, info.iColumn, info.pszSection );
	}
	else
	{
		as::Critical( "%s in unknown function\n", pszAction );
	}
}
