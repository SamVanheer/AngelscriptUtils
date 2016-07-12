#include <algorithm>
#include <cassert>

#include "wrapper/CASContext.h"

#include "CASModule.h"

#include "CASHook.h"

CASHook::CASHook( const char* const pszName, const char* pszArguments, const char* const pszCategory, const asDWORD accessMask, const HookStopMode stopMode )
	: m_pszName( pszName )
	, m_pszArguments( pszArguments )
	, m_pszCategory( pszCategory )
	, m_AccessMask( accessMask )
	, m_StopMode( stopMode )
{
	assert( pszName );
	assert( pszArguments );
	assert( pszCategory );
	assert( accessMask != 0 );
}

size_t CASHook::GetFunctionCount() const
{
	return m_Functions.size();
}

asIScriptFunction* CASHook::GetFunctionByIndex( const size_t uiIndex ) const
{
	assert( uiIndex < m_Functions.size() );

	return m_Functions[ uiIndex ];
}

bool CASHook::AddFunction( asIScriptFunction* pFunction )
{
	assert( pFunction );

	//Don't add functions while this hook is being invoked.
	if( m_iInCallCount != 0 )
	{
		//TODO: log error
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
	});

	return true;
}

void CASHook::RemoveFunction( asIScriptFunction* pFunction )
{
	//Don't remove functions while this hook is being invoked.
	if( m_iInCallCount != 0 )
	{
		//TODO: log error
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

void CASHook::RemoveFunctionsOfModule( CASModule* pModule )
{
	assert( pModule );

	//This method should never be called while in a hook invocation.
	if( m_iInCallCount != 0 )
	{
		assert( !"CASHook::RemoveFunctionsOfModule: Module hooks should not be removed while invoking hooks!" );
		//TODO: log error
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
		//TODO: Delegates need to be double dereferenced for some reason.
		/*
		if( ( *it2 )->GetDelegateFunction() )
			( *it2 )->Release();
			*/

		( *it2 )->Release();
	}

	m_Functions.erase( it, m_Functions.end() );
}

void CASHook::RemoveAllFunctions()
{
	//This method should never be called while in a hook invocation.
	if( m_iInCallCount != 0 )
	{
		assert( !"CASHook::RemoveAllFunctions: Hooks should not be removed while invoking hooks!" );
		//TODO: log error
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

HookCallResult CASHook::VCall( asIScriptContext* pContext, CallFlags_t flags, va_list list )
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
		if( m_StopMode == HookStopMode::MODULE_HANDLED && returnCode == HookReturnCode::HANDLED )
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
			if( m_StopMode == HookStopMode::ON_HANDLED )
				break;
		}
	}

	--m_iInCallCount;

	assert( m_iInCallCount >= 0 );

	if( !bSuccess )
		return HookCallResult::FAILED;

	return returnCode == HookReturnCode::HANDLED ? HookCallResult::HANDLED : HookCallResult::NONE_HANDLED;
}

HookCallResult CASHook::VCall( asIScriptContext* pContext, va_list list )
{
	return VCall( pContext, CallFlag::NONE, list );
}

HookCallResult CASHook::VCall( CallFlags_t flags, va_list list )
{
	if( m_Functions.empty() )
		return HookCallResult::NONE_HANDLED;

	auto pEngine = m_Functions[ 0 ]->GetEngine();
	auto pContext = pEngine->RequestContext();

	auto success = VCall( pContext, flags, list );

	pEngine->ReturnContext( pContext );

	return success;
}

HookCallResult CASHook::Call( asIScriptContext* pContext, CallFlags_t flags, ... )
{
	va_list list;

	va_start( list, flags );

	auto success = VCall( pContext, flags, list );

	va_end( list );

	return success;
}

HookCallResult CASHook::Call( asIScriptContext* pContext, ... )
{
	va_list list;

	va_start( list, pContext );

	auto success = VCall( pContext, list );

	va_end( list );

	return success;
}

HookCallResult CASHook::Call( CallFlags_t flags, ... )
{
	va_list list;

	va_start( list, flags );

	auto success = VCall( flags, list );

	va_end( list );

	return success;
}