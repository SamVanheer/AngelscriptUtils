#include <algorithm>
#include <cassert>
#include <iostream>

#include "wrapper/CASContext.h"

#include "CASModule.h"

#include "CASEvent.h"

CASEvent::CASEvent( const char* const pszName, const char* pszArguments, const char* const pszCategory, const asDWORD accessMask, const EventStopMode stopMode )
	: CASBaseEvent( accessMask, stopMode )
	, m_pszName( pszName )
	, m_pszArguments( pszArguments )
	, m_pszCategory( pszCategory )
{
	assert( pszName );
	assert( pszArguments );
	assert( pszCategory );
}

void CASEvent::DumpHookedFunctions() const
{
	CASBaseEvent::DumpHookedFunctions( ( std::string( "Event\"" ) + GetCategory() + "::" + GetName() + '(' + GetArguments() + ")\"" ).c_str() );
}

void RegisterScriptCEvent( asIScriptEngine& engine )
{
	const char* const pszObjectName = "CEvent";

	engine.RegisterObjectType(
		pszObjectName, 0, asOBJ_REF | asOBJ_NOCOUNT );

	RegisterScriptCBaseEvent<CASEvent>( engine, pszObjectName );
}
