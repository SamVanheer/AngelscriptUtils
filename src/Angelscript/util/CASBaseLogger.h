#ifndef ANGELSCRIPT_UTIL_CASBASELOGGER_H
#define ANGELSCRIPT_UTIL_CASBASELOGGER_H

#include "IASLogger.h"
#include "CASBaseClass.h"

/**
*	Base class for logger implementations that forwards message calls to the main messager handler method.
*	@tparam BASECLASS Class to inherit from. Usually IASLogger, but can be another interface.
*/
template<typename BASECLASS>
class CASBaseLogger : public BASECLASS, CASAtomicRefCountedBaseClass
{
public:
	CASBaseLogger() = default;
	virtual ~CASBaseLogger() = default;

	//Overridden so the correct implementation is called.
	void AddRef() const override
	{
		CASAtomicRefCountedBaseClass::AddRef();
	}

	void Release() const override
	{
		if( CASAtomicRefCountedBaseClass::InternalRelease() )
			delete this;
	}

	void Log( LogLevel_t logLevel, const char* pszFormat, ... ) override
	{
		va_list list;

		va_start( list, pszFormat );

		this->VLog( logLevel, pszFormat, list );

		va_end( list );
	}

	void Critical( const char* pszFormat, ... ) override
	{
		va_list list;

		va_start( list, pszFormat );

		this->VLog( ASLog::CRITICAL, pszFormat, list );

		va_end( list );
	}

	void VCritical( const char* pszFormat, va_list list ) override
	{
		this->VLog( ASLog::CRITICAL, pszFormat, list );
	}

	void Msg( const char* pszFormat, ... ) override
	{
		va_list list;

		va_start( list, pszFormat );

		this->VLog( ASLog::NORMAL, pszFormat, list );

		va_end( list );
	}

	void VMsg( const char* pszFormat, va_list list ) override
	{
		this->VLog( ASLog::NORMAL, pszFormat, list );
	}

	void Verbose( const char* pszFormat, ... ) override
	{
		va_list list;

		va_start( list, pszFormat );

		this->VLog( ASLog::VERBOSE, pszFormat, list );

		va_end( list );
	}

	void VVerbose( const char* pszFormat, va_list list ) override
	{
		this->VLog( ASLog::VERBOSE, pszFormat, list );
	}

	void Diagnostic( const char* pszFormat, ... ) override
	{
		va_list list;

		va_start( list, pszFormat );

		this->VLog( ASLog::DIAGNOSTIC, pszFormat, list );

		va_end( list );
	}

	void VDiagnostic( const char* pszFormat, va_list list ) override
	{
		this->VLog( ASLog::DIAGNOSTIC, pszFormat, list );
	}

private:
	CASBaseLogger( const CASBaseLogger& ) = delete;
	CASBaseLogger& operator=( const CASBaseLogger& ) = delete;
};

#endif //ANGELSCRIPT_UTIL_CASBASELOGGER_H
