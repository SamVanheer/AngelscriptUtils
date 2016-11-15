#ifndef ANGELSCRIPT_UTIL_ASLOGGING_H
#define ANGELSCRIPT_UTIL_ASLOGGING_H

#include "IASLogger.h"

/**
*	@file
*	Defines logging functions.
*/

namespace as
{
/**
*	Gets the current logger, if any.
*/
IASLogger* GetLogger();

/**
*	Sets the current logger. Can be null.
*/
void SetLogger( IASLogger* pLogger );

/**
*	@copydoc IASLogger::Log
*/
void Log( LogLevel_t logLevel, const char* pszFormat, ... );

/**
*	@copydoc IASLogger::VLog
*/
void VLog( LogLevel_t logLevel, const char* pszFormat, va_list list );

/**
*	@copydoc IASLogger::Critical
*/
void Critical( const char* pszFormat, ... );

/**
*	@copydoc IASLogger::VCritical
*/
void VCritical( const char* pszFormat, va_list list );

/**
*	@copydoc IASLogger::Msg
*/
void Msg( const char* pszFormat, ... );

/**
*	@copydoc IASLogger::VMsg
*/
void VMsg( const char* pszFormat, va_list list );

/**
*	@copydoc IASLogger::Verbose
*/
void Verbose( const char* pszFormat, ... );

/**
*	@copydoc IASLogger::VVerbose
*/
void VVerbose( const char* pszFormat, va_list list );

/**
*	@copydoc IASLogger::Diagnostic
*/
void Diagnostic( const char* pszFormat, ... );

/**
*	@copydoc IASLogger::VDiagnostic
*/
void VDiagnostic( const char* pszFormat, va_list list );
}

#endif //ANGELSCRIPT_UTIL_ASLOGGING_H
