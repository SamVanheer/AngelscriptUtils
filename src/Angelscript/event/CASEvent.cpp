#include <algorithm>
#include <cassert>
#include <iostream>

#include "Angelscript/wrapper/CASContext.h"

#include "Angelscript/CASModule.h"

#include "CASEvent.h"

CASEvent::CASEvent( const char* const pszName, const char* pszArguments, const char* const pszCategory, const asDWORD accessMask, const EventStopMode stopMode )
	: CASBaseEvent( accessMask )
	, m_pszName( pszName )
	, m_pszArguments( pszArguments )
	, m_pszCategory( pszCategory )
	, m_StopMode( stopMode )
{
	assert( pszName );
	assert( pszArguments );
	assert( pszCategory );
}

void CASEvent::DumpHookedFunctions() const
{
	CASBaseEvent::DumpHookedFunctions( ( std::string( "Event\"" ) + GetCategory() + "::" + GetName() + '(' + GetArguments() + ")\"" ).c_str() );
}

void RegisterScriptHookReturnCode( asIScriptEngine& engine )
{
	const char* const pszObjectName = "HookReturnCode";

	int result = engine.RegisterEnum( pszObjectName );

	assert( result >= 0 );

	result = engine.RegisterEnumValue( pszObjectName, "HOOK_CONTINUE", static_cast<int>( HookReturnCode::CONTINUE ) );
	assert( result >= 0 );

	result = engine.RegisterEnumValue( pszObjectName, "HOOK_HANDLED", static_cast<int>( HookReturnCode::HANDLED ) );
	assert( result >= 0 );
}

void RegisterScriptCEvent( asIScriptEngine& engine )
{
	const char* const pszObjectName = "CEvent";

	engine.RegisterObjectType(
		pszObjectName, 0, asOBJ_REF | asOBJ_NOCOUNT );

	RegisterScriptCBaseEvent<CASEvent>( engine, pszObjectName );
}
