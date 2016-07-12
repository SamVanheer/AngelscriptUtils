#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>

#include "CASManager.h"

#include "CASHook.h"

#include "CASHookManager.h"

/**
*	Releases a vararg argument.
*	TODO move
*/
static void ReleaseVarArg( asIScriptEngine& engine, void* pObject, const int iTypeId )
{
	if( !pObject )
		return;

	auto pTypeInfo = engine.GetTypeInfoById( iTypeId );

	if( iTypeId & asTYPEID_OBJHANDLE )
	{
		pObject = *reinterpret_cast<void**>( pObject );
	}

	if( iTypeId & asTYPEID_MASK_OBJECT )
	{
		auto pFunction = reinterpret_cast<asIScriptFunction*>( pObject );
		engine.ReleaseScriptObject( pObject, pTypeInfo );
	}
}

void CASHookManager_HookFunction( asIScriptGeneric* pArguments )
{
	pArguments->SetReturnByte( reinterpret_cast<CASHookManager*>( pArguments->GetObject() )->HookFunction( 
		pArguments->GetArgDWord( 0 ), pArguments->GetArgAddress( 1 ), pArguments->GetArgTypeId( 1 ) ) );

	ReleaseVarArg( *pArguments->GetEngine(), pArguments->GetArgAddress( 1 ), pArguments->GetArgTypeId( 1 ) );
}

void CASHookManager_UnhookFunction( asIScriptGeneric* pArguments )
{
	reinterpret_cast<CASHookManager*>( pArguments->GetObject() )->UnhookFunction( pArguments->GetArgDWord( 0 ), pArguments );

	ReleaseVarArg( *pArguments->GetEngine(), pArguments->GetArgAddress( 1 ), pArguments->GetArgTypeId( 1 ) );
}

CASHookManager::CASHookManager( CASManager& manager )
	: m_Manager( manager )
{
}

CASHook* CASHookManager::FindHookByID( const as::HookID_t hookID ) const
{
	if( hookID == as::INVALID_HOOK_ID )
		return nullptr;

	const size_t uiIndex = static_cast<size_t>( hookID - 1 );

	return m_Hooks[ uiIndex ];
}

bool CASHookManager::AddHook( CASHook* pHook )
{
	assert( pHook );

	if( !pHook )
		return false;

	if( m_Hooks.size() >= as::LAST_HOOK_ID )
		return false;

	if( std::find( m_Hooks.begin(), m_Hooks.end(), pHook ) != m_Hooks.end() )
		return true;

	m_Hooks.push_back( pHook );

	pHook->SetHookID( m_Hooks.size() );

	return true;
}

void CASHookManager::RegisterHooks( asIScriptEngine& engine )
{
	std::string szNS;
	std::string szOldNS = engine.GetDefaultNamespace();

	int result = engine.RegisterEnum( "HookReturnCode" );

	assert( result >= 0 );

	result = engine.RegisterEnumValue( "HookReturnCode", "HOOK_CONTINUE", static_cast<int>( HookReturnCode::CONTINUE ) );
	assert( result >= 0 );

	result = engine.RegisterEnumValue( "HookReturnCode", "HOOK_HANDLED", static_cast<int>( HookReturnCode::HANDLED ) );
	assert( result >= 0 );

	asUINT uiHookIndex = engine.GetFuncdefCount();

	std::string szDeclaration;

	for( auto pHook : m_Hooks )
	{
		szNS = "Hooks";

		if( *pHook->GetCategory() )
		{
			szNS += "::";
			szNS += pHook->GetCategory();
		}

		result = engine.SetDefaultNamespace( szNS.c_str() );
		assert( result >= 0 );

		szDeclaration = std::string( "const uint32 " ) + pHook->GetName();
		result = engine.RegisterGlobalProperty( szDeclaration.c_str(), &pHook->GetMutableHookID() );
		assert( result >= 0 );

		result = engine.SetDefaultNamespace( "" );
		assert( result >= 0 );

		szDeclaration = std::string( "HookReturnCode " ) + pHook->GetName() + "Function(" + pHook->GetArguments() + ")";

		result = engine.RegisterFuncdef( szDeclaration.c_str() );
		assert( result >= 0 );

		pHook->SetFuncDef( engine.GetFuncdefByIndex( uiHookIndex )->GetFuncdefSignature() );
	}

	result = engine.SetDefaultNamespace( szOldNS.c_str() );
	assert( result >= 0 );

	engine.RegisterObjectType( "CHookManager", 0, asOBJ_REF | asOBJ_NOCOUNT );

	engine.RegisterObjectMethod( "CHookManager", "bool HookFunction(const uint32 hookID, ?& in)", asMETHOD( CASHookManager, HookFunction ), asCALL_THISCALL );

	engine.RegisterObjectMethod( "CHookManager", "void UnhookFunction(const uint32 hookID, ?& in)", asFUNCTION( CASHookManager_UnhookFunction ), asCALL_GENERIC );

	engine.RegisterGlobalProperty( "CHookManager g_HookManager", this );
}

bool CASHookManager::HookFunction( const as::HookID_t hookID, void* pValue, const int iTypeId )
{
	assert( pValue );

	if( !pValue )
		return false;

	auto pHook = FindHookByID( hookID );

	if( !pHook )
	{
		return false;
	}

	asIScriptFunction* pFunction = nullptr;

	if( !ValidateHookFunction( pHook, iTypeId, pValue, "HookFunction", pFunction ) )
	{
		return false;
	}

	const auto success = pHook->AddFunction( pFunction );

	return success;
}

void CASHookManager::UnhookFunction( const as::HookID_t hookID, asIScriptGeneric* pArguments )
{
	assert( pArguments );

	if( !pArguments )
		return;

	auto pHook = FindHookByID( hookID );

	if( !pHook )
	{
		return;
	}

	asIScriptFunction* pFunction = nullptr;

	const int iTypeId = pArguments->GetArgTypeId( 1 );

	if( !ValidateHookFunction( pHook, pArguments->GetArgTypeId( 1 ), pArguments->GetArgAddress( 1 ), "UnhookFunction", pFunction ) )
	{
		return;
	}

	pHook->RemoveFunction( pFunction );
}

void CASHookManager::UnhookModuleFunctions( CASModule* pModule )
{
	assert( pModule );

	if( !pModule )
		return;

	for( auto pHook : m_Hooks )
	{
		pHook->RemoveFunctionsOfModule( pModule );
	}
}

void CASHookManager::DumpHookedFunctions() const
{
	for( auto pHook : m_Hooks )
	{
		std::cout << "Hook \"" << pHook->GetCategory() << "::" << pHook->GetName() << "(" << pHook->GetArguments() << ")\"" << std::endl;

		for( size_t uiIndex = 0; uiIndex < pHook->GetFunctionCount(); ++uiIndex )
		{
			auto pFunc = pHook->GetFunctionByIndex( uiIndex );

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

bool CASHookManager::ValidateHookFunction( const CASHook* pHook, const int iTypeId, void* pObject, const char* const pszScope, asIScriptFunction*& pOutFunction ) const
{
	pOutFunction = nullptr;

	asITypeInfo* pObjectType = m_Manager.GetEngine()->GetTypeInfoById( iTypeId );

	if( !pObjectType )
	{
		//TODO
		//gASLog()->Error( ASLOG_CRITICAL, "CModuleHookManager::%s: unknown type!\n", pszScope );
		return false;
	}

	if( !( pObjectType->GetFlags() & asOBJ_FUNCDEF ) )
	{
		//TODO
		//gASLog()->Error( ASLOG_CRITICAL, "CModuleHookManager::%s: Object is not a function or delegate!\n", pszScope );
		return false;
	}

	if( !pObject )
	{
		//TODO
		//gASLog()->Error( ASLOG_CRITICAL, "CModuleHookManager::%s: Object is null!\n", pszScope );
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
		//gASLog()->Error( ASLOG_CRITICAL, "CModuleHookManager::%s: Object is null!\n", pszScope );
		return false;
	}

	asIScriptFunction* pFunction = reinterpret_cast<asIScriptFunction*>( pObject );

	if( !pFunction )
	{
		//TODO
		//gASLog()->Error( ASLOG_CRITICAL, "CModuleHookManager::%s: Null function passed!\n", pszScope );
		return false;
	}

	asIScriptFunction* const pFuncDef = pHook->GetFuncDef();

	//Verify the function format
	if( !pFuncDef->IsCompatibleWithTypeId( pFunction->GetTypeId() ) )
	{
		if( asIScriptFunction* pDelegate = pFunction->GetDelegateFunction() )
		{
			asITypeInfo* pDelegateTypeInfo = pFunction->GetDelegateObjectType();

			//TODO
			/*
			gASLog()->Error( ASLOG_CRITICAL, "CModuleHookManager::%s: Method '%s::%s::%s' is incompatible with hook '%s'!\n",
							 pszScope, pDelegateObjectType->GetNamespace(), pDelegateObjectType->GetName(), pDelegate->GetName(), pFuncDef->GetName() );
							 */
		}
		else
		{
			//TODO
			/*
			gASLog()->Error( ASLOG_CRITICAL, "CModuleHookManager::%s: Function '%s::%s' is incompatible with hook '%s'!\n",
							 pszScope, pFunction->GetNamespace(), pFunction->GetName(), pFuncDef->GetName() );
							 */
		}

		return false;
	}

	pOutFunction = pFunction;

	return true;
}