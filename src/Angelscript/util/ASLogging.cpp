#include "Angelscript/util/ASUtil.h"

#include "ASLogging.h"

namespace as
{
namespace
{
IASLogger* g_pLogger = nullptr;
}

IASLogger* GetLogger()
{
	return g_pLogger;
}

void SetLogger( IASLogger* pLogger )
{
	as::SetRefPointer( g_pLogger, pLogger );
}

void Log( LogLevel_t logLevel, const char* pszFormat, ... )
{
	if( !g_pLogger )
		return;

	va_list list;

	va_start( list, pszFormat );

	g_pLogger->VLog( logLevel, pszFormat, list );

	va_end( list );
}

void VLog( LogLevel_t logLevel, const char* pszFormat, va_list list )
{
	if( !g_pLogger )
		return;

	g_pLogger->VLog( logLevel, pszFormat, list );
}

void Critical( const char* pszFormat, ... )
{
	if( !g_pLogger )
		return;

	va_list list;

	va_start( list, pszFormat );

	g_pLogger->VCritical( pszFormat, list );

	va_end( list );
}

void VCritical( const char* pszFormat, va_list list )
{
	if( !g_pLogger )
		return;

	g_pLogger->VCritical( pszFormat, list );
}

void Msg( const char* pszFormat, ... )
{
	if( !g_pLogger )
		return;

	va_list list;

	va_start( list, pszFormat );

	g_pLogger->VMsg( pszFormat, list );

	va_end( list );
}

void VMsg( const char* pszFormat, va_list list )
{
	if( !g_pLogger )
		return;

	g_pLogger->VMsg( pszFormat, list );
}

void Verbose( const char* pszFormat, ... )
{
	if( !g_pLogger )
		return;

	va_list list;

	va_start( list, pszFormat );

	g_pLogger->VVerbose( pszFormat, list );

	va_end( list );
}

void VVerbose( const char* pszFormat, va_list list )
{
	if( !g_pLogger )
		return;

	g_pLogger->VVerbose( pszFormat, list );
}

void Diagnostic( const char* pszFormat, ... )
{
	if( !g_pLogger )
		return;

	va_list list;

	va_start( list, pszFormat );

	g_pLogger->VDiagnostic( pszFormat, list );

	va_end( list );
}

void VDiagnostic( const char* pszFormat, va_list list )
{
	if( !g_pLogger )
		return;

	g_pLogger->VDiagnostic( pszFormat, list );
}
}
