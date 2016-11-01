#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>

#include "util/ASUtil.h"

#include "CASManager.h"

#include "CASEvent.h"

#include "CASEventManager.h"

CASEventManager::CASEventManager( CASManager& manager )
	: m_Manager( manager )
{
}

CASEventManager::~CASEventManager()
{
	UnhookAllFunctions();
}

CASEvent* CASEventManager::FindEventByName( const std::string& szName )
{
	auto szNamespace = as::ExtractNamespaceFromName( szName );
	const auto szEventName = as::ExtractNameFromName( szName );

	const auto index = szNamespace.find( "Events::" );

	//If the user specified Events:: as the namespace, strip it.
	if( index != std::string::npos )
	{
		szNamespace = szNamespace.substr( strlen( "Events::" ) );
	}

	for( auto pEvent : m_Events )
	{
		if( szNamespace == pEvent->GetCategory() && 
			szEventName == pEvent->GetName() )
			return pEvent;
	}

	return nullptr;
}

bool CASEventManager::AddEvent( CASEvent* pEvent )
{
	assert( pEvent );

	if( !pEvent )
		return false;

	if( std::find( m_Events.begin(), m_Events.end(), pEvent ) != m_Events.end() )
		return true;

	m_Events.push_back( pEvent );

	return true;
}

void CASEventManager::RegisterEvents( asIScriptEngine& engine )
{
	std::string szOldNS = engine.GetDefaultNamespace();

	const asDWORD accessMask = engine.SetDefaultAccessMask( 0xFFFFFFFF );

	engine.RegisterGlobalProperty( "CEventManager g_EventManager", this );

	asUINT uiHookIndex = engine.GetFuncdefCount();

	std::string szNS;
	std::string szDeclaration;

	int result;

	for( auto pEvent : m_Events )
	{
		szNS = "Events";

		if( *pEvent->GetCategory() )
		{
			szNS += "::";
			szNS += pEvent->GetCategory();
		}

		result = engine.SetDefaultNamespace( szNS.c_str() );
		assert( result >= 0 );

		engine.SetDefaultAccessMask( pEvent->GetAccessMask() );

		szDeclaration = std::string( "::CEvent " ) + pEvent->GetName();

		engine.RegisterGlobalProperty( szDeclaration.c_str(), pEvent );

		result = engine.SetDefaultNamespace( "" );
		assert( result >= 0 );

		szDeclaration = std::string( "HookReturnCode " ) + pEvent->GetName() + "Hook(" + pEvent->GetArguments() + ")";

		result = engine.RegisterFuncdef( szDeclaration.c_str() );
		assert( result >= 0 );

		pEvent->SetFuncDef( engine.GetFuncdefByIndex( uiHookIndex++ )->GetFuncdefSignature() );
	}

	result = engine.SetDefaultNamespace( szOldNS.c_str() );
	assert( result >= 0 );

	engine.SetDefaultAccessMask( accessMask );
}

void CASEventManager::UnhookModuleFunctions( CASModule* pModule )
{
	assert( pModule );

	if( !pModule )
		return;

	for( auto pEvent : m_Events )
	{
		pEvent->RemoveFunctionsOfModule( pModule );
	}
}

void CASEventManager::UnhookAllFunctions()
{
	for( auto pEvent : m_Events )
	{
		pEvent->RemoveAllFunctions();
	}
}

void CASEventManager::DumpHookedFunctions() const
{
	for( auto pEvent : m_Events )
	{
		pEvent->DumpHookedFunctions();
	}
}

static void RegisterScriptHookReturnCode( asIScriptEngine& engine )
{
	const char* const pszObjectName = "HookReturnCode";

	int result = engine.RegisterEnum( pszObjectName );

	assert( result >= 0 );

	result = engine.RegisterEnumValue( pszObjectName, "HOOK_CONTINUE", static_cast<int>( HookReturnCode::CONTINUE ) );
	assert( result >= 0 );

	result = engine.RegisterEnumValue( pszObjectName, "HOOK_HANDLED", static_cast<int>( HookReturnCode::HANDLED ) );
	assert( result >= 0 );
}

static void RegisterScriptCEventManager( asIScriptEngine& engine )
{
	const char* const pszObjectName = "CEventManager";

	engine.RegisterObjectType( pszObjectName, 0, asOBJ_REF | asOBJ_NOCOUNT );

	engine.RegisterObjectMethod(
		pszObjectName, "CEvent@ FindEventByName(const string& in szName)",
		asMETHOD( CASEventManager, FindEventByName ), asCALL_THISCALL );
}

void RegisterScriptEventAPI( asIScriptEngine& engine )
{
	const asDWORD accessMask = engine.SetDefaultAccessMask( 0xFFFFFFFF );

	RegisterScriptHookReturnCode( engine );
	RegisterScriptCEvent( engine );
	RegisterScriptCEventManager( engine );

	engine.SetDefaultAccessMask( accessMask );
}