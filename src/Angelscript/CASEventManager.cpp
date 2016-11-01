#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>

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

CASEvent* CASEventManager::FindEventByID( const as::EventID_t eventID ) const
{
	if( eventID == as::INVALID_EVENT_ID )
		return nullptr;

	const size_t uiIndex = static_cast<size_t>( eventID - 1 );

	return m_Events[ uiIndex ];
}

bool CASEventManager::AddEvent( CASEvent* pEvent )
{
	assert( pEvent );

	if( !pEvent )
		return false;

	if( m_Events.size() >= as::LAST_EVENT_ID )
		return false;

	if( std::find( m_Events.begin(), m_Events.end(), pEvent ) != m_Events.end() )
		return true;

	m_Events.push_back( pEvent );

	pEvent->SetEventID( m_Events.size() );

	return true;
}

void CASEventManager::RegisterEvents( asIScriptEngine& engine )
{
	std::string szNS;
	std::string szOldNS = engine.GetDefaultNamespace();

	const asDWORD accessMask = engine.SetDefaultAccessMask( 0xFFFFFFFF );

	int result = engine.RegisterEnum( "HookReturnCode" );

	assert( result >= 0 );

	result = engine.RegisterEnumValue( "HookReturnCode", "HOOK_CONTINUE", static_cast<int>( HookReturnCode::CONTINUE ) );
	assert( result >= 0 );

	result = engine.RegisterEnumValue( "HookReturnCode", "HOOK_HANDLED", static_cast<int>( HookReturnCode::HANDLED ) );
	assert( result >= 0 );

	engine.RegisterObjectType( "CEventManager", 0, asOBJ_REF | asOBJ_NOCOUNT );

	engine.RegisterObjectMethod( "CEventManager", "bool HookFunction(const uint32 eventID, ?& in)", asMETHOD( CASEventManager, HookFunction ), asCALL_THISCALL );

	engine.RegisterObjectMethod( "CEventManager", "void UnhookFunction(const uint32 eventID, ?& in)", asMETHOD( CASEventManager, UnhookFunction ), asCALL_THISCALL );

	engine.RegisterGlobalProperty( "CEventManager g_EventManager", this );

	asUINT uiHookIndex = engine.GetFuncdefCount();

	std::string szDeclaration;

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

		szDeclaration = std::string( "const uint32 " ) + pEvent->GetName();
		result = engine.RegisterGlobalProperty( szDeclaration.c_str(), &pEvent->GetMutableEventID() );
		assert( result >= 0 );

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

bool CASEventManager::HookFunction( const as::EventID_t eventID, void* pValue, const int iTypeId )
{
	assert( pValue );

	if( !pValue )
		return false;

	auto pEvent = FindEventByID( eventID );

	if( !pEvent )
	{
		return false;
	}

	asIScriptFunction* pFunction = nullptr;

	if( !ValidateHookFunction( pEvent, iTypeId, pValue, "HookFunction", pFunction ) )
	{
		return false;
	}

	const auto success = pEvent->AddFunction( pFunction );

	return success;
}

void CASEventManager::UnhookFunction( const as::EventID_t eventID, void* pValue, const int iTypeId )
{
	assert( pValue );

	if( !pValue )
		return;

	auto pEvent = FindEventByID( eventID );

	if( !pEvent )
	{
		return;
	}

	asIScriptFunction* pFunction = nullptr;

	if( !ValidateHookFunction( pEvent, iTypeId, pValue, "UnhookFunction", pFunction ) )
	{
		return;
	}

	pEvent->RemoveFunction( pFunction );
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
		std::cout << "Event \"" << pEvent->GetCategory() << "::" << pEvent->GetName() << "(" << pEvent->GetArguments() << ")\"" << std::endl;

		for( size_t uiIndex = 0; uiIndex < pEvent->GetFunctionCount(); ++uiIndex )
		{
			auto pFunc = pEvent->GetFunctionByIndex( uiIndex );

			auto pModule = pFunc->GetModule();

			decltype( pFunc ) pActualFunc = pFunc;

			if( !pModule )
			{
				auto pDelegate = pFunc->GetDelegateFunction();

				if( pDelegate )
				{
					pActualFunc = pDelegate;
					pModule = pDelegate->GetModule();
				}
			}

			if( !pActualFunc )
			{
				std::cout << "Null function!" << std::endl;
			}

			if( !pModule )
			{
				std::cout << "Null module!" << std::endl;
				continue;
			}

			std::cout << "Module \"" << pModule->GetName() << "\", \"" << pActualFunc->GetNamespace() << "::" << pActualFunc->GetName() << "\"" << std::endl;
		}

		std::cout << "End functions" << std::endl;
	}
}

bool CASEventManager::ValidateHookFunction( const CASEvent* pEvent, const int iTypeId, void* pObject, const char* const pszScope, asIScriptFunction*& pOutFunction ) const
{
	pOutFunction = nullptr;

	asITypeInfo* pObjectType = m_Manager.GetEngine()->GetTypeInfoById( iTypeId );

	if( !pObjectType )
	{
		//TODO
		//gASLog()->Error( ASLOG_CRITICAL, "CASEventManager::%s: unknown type!\n", pszScope );
		return false;
	}

	if( !( pObjectType->GetFlags() & asOBJ_FUNCDEF ) )
	{
		//TODO
		//gASLog()->Error( ASLOG_CRITICAL, "CASEventManager::%s: Object is not a function or delegate!\n", pszScope );
		return false;
	}

	if( !pObject )
	{
		//TODO
		//gASLog()->Error( ASLOG_CRITICAL, "CASEventManager::%s: Object is null!\n", pszScope );
		return false;
	}

	//pObjectType->GetTypeId() is -1 for some reason
	if( iTypeId & asTYPEID_OBJHANDLE )
	{
		pObject = *reinterpret_cast<void**>( pObject );
	}

	if( !pObject )
	{
		//TODO
		//gASLog()->Error( ASLOG_CRITICAL, "CASEventManager::%s: Object is null!\n", pszScope );
		return false;
	}

	asIScriptFunction* pFunction = reinterpret_cast<asIScriptFunction*>( pObject );

	if( !pFunction )
	{
		//TODO
		//gASLog()->Error( ASLOG_CRITICAL, "CASEventManager::%s: Null function passed!\n", pszScope );
		return false;
	}

	asIScriptFunction* const pFuncDef = pEvent->GetFuncDef();

	//Verify the function format
	if( !pFuncDef->IsCompatibleWithTypeId( pFunction->GetTypeId() ) )
	{
		if( asIScriptFunction* pDelegate = pFunction->GetDelegateFunction() )
		{
			asITypeInfo* pDelegateTypeInfo = pFunction->GetDelegateObjectType();

			//TODO
			/*
			gASLog()->Error( ASLOG_CRITICAL, "CASEventManager::%s: Method '%s::%s::%s' is incompatible with event '%s'!\n",
							 pszScope, pDelegateObjectType->GetNamespace(), pDelegateObjectType->GetName(), pDelegate->GetName(), pFuncDef->GetName() );
							 */
		}
		else
		{
			//TODO
			/*
			gASLog()->Error( ASLOG_CRITICAL, "CASEventManager::%s: Function '%s::%s' is incompatible with event '%s'!\n",
							 pszScope, pFunction->GetNamespace(), pFunction->GetName(), pFuncDef->GetName() );
							 */
		}

		return false;
	}

	pOutFunction = pFunction;

	return true;
}