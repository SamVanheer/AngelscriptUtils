#include <algorithm>
#include <iostream>

#include "util/ASLogging.h"

#include "CASModule.h"

#include "CASBaseEvent.h"

CASBaseEvent::CASBaseEvent( const asDWORD accessMask, const EventStopMode stopMode )
	: m_AccessMask( accessMask )
	, m_StopMode( stopMode )
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
	if( m_iInCallCount != 0 )
	{
		const char* pszFile = nullptr;
		int iLine = 0;
		int iColumn = 0;

		if( auto pContext = asGetActiveContext() )
		{
			iLine = pContext->GetLineNumber( 0, &iColumn, &pszFile );
		}

		if( !pszFile )
			pszFile = "Unknown";

		as::Critical( "CBaseEvent::AddFunction: %s(%d, %d): Cannot add function while invoking event!\n", pszFile, iLine, iColumn );
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
	//Don't remove functions while this event is being invoked.
	if( m_iInCallCount != 0 )
	{
		const char* pszFile = nullptr;
		int iLine = 0;
		int iColumn = 0;

		if( auto pContext = asGetActiveContext() )
		{
			iLine = pContext->GetLineNumber( 0, &iColumn, &pszFile );
		}

		if( !pszFile )
			pszFile = "Unknown";

		as::Critical( "CBaseEvent::RemoveFunction: %s(%d, %d): Cannot remove function while invoking event!\n", pszFile, iLine, iColumn );
		return;
	}

	if( !pFunction )
		return;

	auto it = std::find( m_Functions.begin(), m_Functions.end(), pFunction );

	if( it == m_Functions.end() )
		return;

	( *it )->Release();

	m_Functions.erase( it );
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
	if( m_iInCallCount != 0 )
	{
		assert( !"CBaseEvent::RemoveFunctionsOfModule: Module hooks should not be removed while invoking events!" );

		const char* pszFile = nullptr;
		int iLine = 0;
		int iColumn = 0;

		if( auto pContext = asGetActiveContext() )
		{
			iLine = pContext->GetLineNumber( 0, &iColumn, &pszFile );
		}

		if( !pszFile )
			pszFile = "Unknown";

		as::Critical( "CBaseEvent::RemoveFunctionsOfModule: %s(%d, %d): Module hooks should not be removed while invoking events!\n", pszFile, iLine, iColumn );
		return;
	}

	if( !pModule )
		return;

	auto it = std::remove_if( m_Functions.begin(), m_Functions.end(), [ = ]( const asIScriptFunction* pFunction )
	{
		auto pOther = GetModuleFromScriptFunction( pFunction );
		return pModule == GetModuleFromScriptFunction( pFunction );
	} );

	for( auto it2 = it; it2 < m_Functions.end(); ++it2 )
	{
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

HookCallResult CASBaseEvent::VCall( asIScriptContext* pContext, CallFlags_t flags, va_list list )
{
	assert( pContext );

	if( !pContext )
		return HookCallResult::FAILED;

	CASContext ctx( *pContext );

	bool bSuccess = true;

	HookReturnCode returnCode = HookReturnCode::CONTINUE;

	asIScriptModule* pLastModule = nullptr;

	++m_iInCallCount;

	for( auto pFunc : m_Functions )
	{
		if( m_StopMode == EventStopMode::MODULE_HANDLED && returnCode == HookReturnCode::HANDLED )
		{
			//A hook in the last module handled it, so stop.
			if( pLastModule && pLastModule != pFunc->GetModule() )
				break;
		}

		pLastModule = pFunc->GetModule();

		CASFunction func( *pFunc, ctx );

		const auto successCall = func.VCall( flags, list );

		bSuccess = successCall && bSuccess;

		//Only check if a HANDLED value was returned if we're still continuing.
		if( successCall && returnCode == HookReturnCode::CONTINUE )
		{
			bSuccess = func.GetReturnValue( &returnCode ) && bSuccess;
		}

		if( returnCode == HookReturnCode::HANDLED )
		{
			if( m_StopMode == EventStopMode::ON_HANDLED )
				break;
		}
	}

	--m_iInCallCount;

	assert( m_iInCallCount >= 0 );

	if( !bSuccess )
		return HookCallResult::FAILED;

	return returnCode == HookReturnCode::HANDLED ? HookCallResult::HANDLED : HookCallResult::NONE_HANDLED;
}

HookCallResult CASBaseEvent::VCall( asIScriptContext* pContext, va_list list )
{
	return VCall( pContext, CallFlag::NONE, list );
}

HookCallResult CASBaseEvent::VCall( CallFlags_t flags, va_list list )
{
	if( m_Functions.empty() )
		return HookCallResult::NONE_HANDLED;

	auto pEngine = m_Functions[ 0 ]->GetEngine();
	auto pContext = pEngine->RequestContext();

	auto success = VCall( pContext, flags, list );

	pEngine->ReturnContext( pContext );

	return success;
}

HookCallResult CASBaseEvent::Call( asIScriptContext* pContext, CallFlags_t flags, ... )
{
	va_list list;

	va_start( list, flags );

	auto success = VCall( pContext, flags, list );

	va_end( list );

	return success;
}

HookCallResult CASBaseEvent::Call( asIScriptContext* pContext, ... )
{
	va_list list;

	va_start( list, pContext );

	auto success = VCall( pContext, list );

	va_end( list );

	return success;
}

HookCallResult CASBaseEvent::Call( CallFlags_t flags, ... )
{
	va_list list;

	va_start( list, flags );

	auto success = VCall( flags, list );

	va_end( list );

	return success;
}

void CASBaseEvent::DumpHookedFunctions( const char* const pszName ) const
{
	if( pszName )
		std::cout << pszName << std::endl;

	for( size_t uiIndex = 0; uiIndex < GetFunctionCount(); ++uiIndex )
	{
		auto pFunc = GetFunctionByIndex( uiIndex );

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
			continue;
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

void RegisterScriptCBaseEvent( asIScriptEngine& engine )
{
	const char* const pszObjectName = "CBaseEvent";

	engine.RegisterObjectType(
		pszObjectName, 0, asOBJ_REF | asOBJ_NOCOUNT );

	RegisterScriptCBaseEvent<CASBaseEvent>( engine, pszObjectName );
}
