#ifndef ANGELSCRIPT_UTIL_IASLOGGER_H
#define ANGELSCRIPT_UTIL_IASLOGGER_H

#include <cstdarg>
#include <cstdint>

using LogLevel_t = int32_t;

namespace ASLog
{
/**
*	Log levels. The implementer may choose to add more log levels if they choose to.
*/
enum ASLog : LogLevel_t
{
	/**
	*	Always log these.
	*/
	CRITICAL = 0,

	/**
	*	Normal logging. Critical messages, user notifications.
	*/
	NORMAL,

	/**
	*	Include events that occur during normal operation that provide additional information.
	*/
	VERBOSE,

	/**
	*	Diagnostics info.
	*/
	DIAGNOSTIC
};

/**
*	Returns the string representation of a log level.
*/
const char* ToString( const ASLog logLevel );
}

/**
*	Interface for a logger for Angelscript.
*/
class IASLogger
{
public:
	virtual ~IASLogger() = 0;

	/**
	*	Called when the logger is being removed.
	*/
	virtual void Release() = 0;

	/**
	*	Logs a message if the given log level is enabled.
	*	@param logLevel Log level.
	*	@param pszFormat Format string.
	*	@param ... Arguments.
	*/
	virtual void Log( LogLevel_t logLevel, const char* pszFormat, ... ) = 0;

	/**
	*	Logs a message if the given log level is enabled.
	*	@param logLevel Log level.
	*	@param pszFormat Format string.
	*	@param list List of arguments to format with.
	*/
	virtual void VLog( LogLevel_t logLevel, const char* pszFormat, va_list list ) = 0;

	/**
	*	Logs a critical message.
	*	@param logLevel Log level.
	*	@param pszFormat Format string.
	*	@param ... Arguments.
	*/
	virtual void Critical( const char* pszFormat, ... ) = 0;

	/**
	*	Logs a critical message.
	*	@param logLevel Log level.
	*	@param pszFormat Format string.
	*	@param list List of arguments to format with.
	*/
	virtual void VCritical( const char* pszFormat, va_list list ) = 0;

	/**
	*	Logs a normal message.
	*	@param logLevel Log level.
	*	@param pszFormat Format string.
	*	@param ... Arguments.
	*/
	virtual void Msg( const char* pszFormat, ... ) = 0;

	/**
	*	Logs a normal message.
	*	@param logLevel Log level.
	*	@param pszFormat Format string.
	*	@param list List of arguments to format with.
	*/
	virtual void VMsg( const char* pszFormat, va_list list ) = 0;

	/**
	*	Logs a verbose message.
	*	@param logLevel Log level.
	*	@param pszFormat Format string.
	*	@param ... Arguments.
	*/
	virtual void Verbose( const char* pszFormat, ... ) = 0;

	/**
	*	Logs a verbose message.
	*	@param logLevel Log level.
	*	@param pszFormat Format string.
	*	@param list List of arguments to format with.
	*/
	virtual void VVerbose( const char* pszFormat, va_list list ) = 0;

	/**
	*	Logs a diagnostic message.
	*	@param logLevel Log level.
	*	@param pszFormat Format string.
	*	@param ... Arguments.
	*/
	virtual void Diagnostic( const char* pszFormat, ... ) = 0;

	/**
	*	Logs a diagnostic message.
	*	@param logLevel Log level.
	*	@param pszFormat Format string.
	*	@param list List of arguments to format with.
	*/
	virtual void VDiagnostic( const char* pszFormat, va_list list ) = 0;
};

inline IASLogger::~IASLogger()
{
}

#endif //ANGELSCRIPT_UTIL_IASLOGGER_H
