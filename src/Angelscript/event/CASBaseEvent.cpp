#include <algorithm>
#include <cassert>

#include "Angelscript/util/ASLogging.h"

#include "Angelscript/CASModule.h"

#include "CASBaseEvent.h"

CASBaseEvent::CASBaseEvent( const asDWORD accessMask )
	: m_AccessMask( accessMask )
{
	assert( accessMask != 0 );
}

CASBaseEvent::~CASBaseEvent()
{
	RemoveAllFunctions();
}

size_t CASBaseEvent::GetFunctionCount() const
{
	return m_Functions.size();
}

asIScriptFunction* CASBaseEvent::GetFunctionByIndex( const size_t uiIndex ) const
{
	assert( uiIndex < m_Functions.size() );

	return m_Functions[ uiIndex ];
}

bool CASBaseEvent::AddFunction( asIScriptFunction* pFunction )
{
	assert( pFunction );

	//Don't add functions while this event is being invoked.
	if( IsTriggering() )
	{
		as::CASCallerInfo info;

		as::GetCallerInfo( info );

		as::Critical( "CBaseEvent::AddFunction: %s(%d, %d): Cannot add function while invoking event!\n", info.pszSection, info.iLine, info.iColumn );
		return false;
	}

	if( !pFunction )
		return false;

	if( std::find( m_Functions.begin(), m_Functions.end(), pFunction ) != m_Functions.end() )
		return true;

	m_Functions.push_back( pFunction );

	pFunction->AddRef();

	std::stable_sort( m_Functions.begin(), m_Functions.end(), []( const asIScriptFunction* pLHS, const asIScriptFunction* pRHS )
	{
		auto pLHSModule = GetModuleFromScriptFunction( pLHS );
		auto pRHSModule = GetModuleFromScriptFunction( pRHS );

		return ModuleLess( pLHSModule, pRHSModule );
	} );

	return true;
}

bool CASBaseEvent::Hook( void* pValue, const int iTypeId )
{
	assert( pValue );

	if( !pValue )
		return false;

	asIScriptFunction* pFunction = nullptr;

	if( !ValidateHookFunction( iTypeId, pValue, "HookFunction", pFunction ) )
	{
		return false;
	}

	return AddFunction( pFunction );
}

void CASBaseEvent::RemoveFunction( asIScriptFunction* pFunction )
{
	if( !pFunction )
		return;

	auto it = std::find( m_Functions.begin(), m_Functions.end(), pFunction );

	if( it == m_Functions.end() )
		return;

	( *it )->Release();

	if( IsTriggering() )
	{
		//Currently triggering, mark as removed.
		*it = nullptr;
	}
	else
	{
		//Remove now.
		m_Functions.erase( it );
	}
}

void CASBaseEvent::Unhook( void* pValue, const int iTypeId )
{
	assert( pValue );

	if( !pValue )
		return;

	asIScriptFunction* pFunction = nullptr;

	if( !ValidateHookFunction( iTypeId, pValue, "UnhookFunction", pFunction ) )
	{
		return;
	}

	RemoveFunction( pFunction );
}

void CASBaseEvent::RemoveFunctionsOfModule( CASModule* pModule )
{
	assert( pModule );

	//This method should never be called while in an event invocation.
	if( IsTriggering() )
	{
		assert( !"CBaseEvent::RemoveFunctionsOfModule: Module hooks should not be removed while invoking events!" );

		as::CASCallerInfo info;

		as::GetCallerInfo( info );

		as::Critical( "CBaseEvent::RemoveFunctionsOfModule: %s(%d, %d): Module hooks should not be removed while invoking events!\n", info.pszSection, info.iLine, info.iColumn );
		return;
	}

	if( !pModule )
		return;

	//These functions might be null in some edge cases due to hooks being removed in event calls.
	auto it = std::remove_if( m_Functions.begin(), m_Functions.end(), [ = ]( const asIScriptFunction* pFunction )
	{
		return !pFunction || pModule == GetModuleFromScriptFunction( pFunction );
	} );

	for( auto it2 = it; it2 < m_Functions.end(); ++it2 )
	{
		if( *it2 )
			( *it2 )->Release();
	}

	m_Functions.erase( it, m_Functions.end() );
}

void CASBaseEvent::RemoveAllFunctions()
{
	//This method should never be called while in an event invocation.
	if( m_iInCallCount != 0 )
	{
		assert( !"CASEvent::RemoveAllFunctions: Hooks should not be removed while invoking events!" );
		as::Critical( "CASEvent::RemoveAllFunctions: Hooks should not be removed while invoking events!\n" );

		if( auto pContext = asGetActiveContext() )
		{
			pContext->SetException( "Cannot remove event hooks during the event's invocation" );
		}

		return;
	}

	for( auto pFunc : m_Functions )
	{
		if( pFunc->GetDelegateFunction() )
			pFunc->Release();

		pFunc->Release();
	}

	m_Functions.clear();
}

bool CASBaseEvent::ValidateHookFunction( const int iTypeId, void* pObject, const char* const pszScope, asIScriptFunction*& pOutFunction ) const
{
	auto pEngine = asGetActiveContext()->GetEngine();

	pOutFunction = nullptr;

	asITypeInfo* pObjectType = pEngine->GetTypeInfoById( iTypeId );

	if( !pObjectType )
	{
		as::Critical( "CBaseEvent::%s: unknown type!\n", pszScope );
		return false;
	}

	if( !( pObjectType->GetFlags() & asOBJ_FUNCDEF ) )
	{
		as::Critical( "CBaseEvent::%s: Object is not a function or delegate!\n", pszScope );
		return false;
	}

	if( !pObject )
	{
		as::Critical( "CBaseEvent::%s: Object is null!\n", pszScope );
		return false;
	}

	//pObjectType->GetTypeId() is -1 for some reason
	if( iTypeId & asTYPEID_OBJHANDLE )
	{
		pObject = *reinterpret_cast<void**>( pObject );
	}

	if( !pObject )
	{
		as::Critical( "CBaseEvent::%s: Object is null!\n", pszScope );
		return false;
	}

	asIScriptFunction* pFunction = reinterpret_cast<asIScriptFunction*>( pObject );

	if( !pFunction )
	{
		as::Critical( "CBaseEvent::%s: Null function passed!\n", pszScope );
		return false;
	}

	asIScriptFunction* const pFuncDef = GetFuncDef();

	assert( pFuncDef );

	if( !pFuncDef )
	{
		as::Critical( "CBaseEvent::%s: No funcdef for event!\n", pszScope );
		return false;
	}

	//Verify the function format
	if( !pFuncDef->IsCompatibleWithTypeId( pFunction->GetTypeId() ) )
	{
		if( asIScriptFunction* pDelegate = pFunction->GetDelegateFunction() )
		{
			asITypeInfo* pDelegateTypeInfo = pFunction->GetDelegateObjectType();

			as::Critical( "CBaseEvent::%s: Method '%s::%s::%s' is incompatible with event '%s'!\n",
				pszScope, pDelegateTypeInfo->GetNamespace(), pDelegateTypeInfo->GetName(), pDelegate->GetName(), pFuncDef->GetName() );
		}
		else
		{
			as::Critical( "CBaseEvent::%s: Function '%s::%s' is incompatible with event '%s'!\n",
				pszScope, pFunction->GetNamespace(), pFunction->GetName(), pFuncDef->GetName() );
		}

		return false;
	}

	pOutFunction = pFunction;

	return true;
}

void CASBaseEvent::DumpHookedFunctions( const char* const pszName ) const
{
	if( pszName )
		as::Msg( "%s\n", pszName );

	for( size_t uiIndex = 0; uiIndex < GetFunctionCount(); ++uiIndex )
	{
		auto pFunc = GetFunctionByIndex( uiIndex );

		auto pModule = pFunc->GetModule();

		auto pActualFunc = pFunc;

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
			as::Msg( "Null function!\n" );
			continue;
		}

		if( !pModule )
		{
			as::Msg( "Null module!\n" );
			continue;
		}

		as::Msg( "Module \"%s\", \"%s::%s\"\n", pModule->GetName(), pActualFunc->GetNamespace(), pActualFunc->GetName() );
	}

	as::Msg( "End functions\n" );
}

void CASBaseEvent::ClearRemovedHooks()
{
	//Can happen when recursively triggering events.
	if( IsTriggering() )
		return;

	for( auto it = m_Functions.begin(); it != m_Functions.end(); )
	{
		if( !( *it ) )
		{
			it = m_Functions.erase( it );
		}
		else
		{
			++it;
		}
	}
}

void RegisterScriptCBaseEvent( asIScriptEngine& engine )
{
	const char* const pszObjectName = "CBaseEvent";

	engine.RegisterObjectType(
		pszObjectName, 0, asOBJ_REF | asOBJ_NOCOUNT );

	RegisterScriptCBaseEvent<CASBaseEvent>( engine, pszObjectName );
}
