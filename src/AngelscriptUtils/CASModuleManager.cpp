#include <algorithm>
#include <cassert>
#include <memory>

#include "AngelscriptUtils/add_on/scriptbuilder/scriptbuilder.h"

#include "AngelscriptUtils/CASModule.h"

#include "AngelscriptUtils/IASModuleBuilder.h"

#include "AngelscriptUtils/CASModuleManager.h"

CASModuleManager::CASModuleManager( asIScriptEngine& engine )
	: m_Engine( engine )
{
	m_Engine.AddRef();
}

CASModuleManager::~CASModuleManager()
{
	m_Engine.Release();
}

/*
*	Include callback for builders.
*/
static int CASModuleManager_IncludeCallback( const char* pszFileName, const char* pszFrom, CScriptBuilder* pBuilder, void* pUserParam )
{
	return reinterpret_cast<IASModuleBuilder*>( pUserParam )->IncludeScript( *pBuilder, pszFileName, pszFrom ) ? 0 : -1;
}

CASModule* CASModuleManager::BuildModule(const char* const pszModuleName, const asDWORD accessMask, IASModuleBuilder& builder)
{
	assert( pszModuleName );

	if( !pszModuleName )
		return nullptr;

	assert( *pszModuleName );

	if( !( *pszModuleName ) )
		return nullptr;

	CScriptBuilder scriptBuilder;

	scriptBuilder.SetIncludeCallback( &::CASModuleManager_IncludeCallback, &builder );

	auto result = scriptBuilder.StartNewModule( &m_Engine, pszModuleName );

	if( result < 0 )
	{
		return nullptr;
	}

	auto pScriptModule = scriptBuilder.GetModule();

	pScriptModule->SetAccessMask(accessMask);

	struct CleanupModuleOnExit final
	{
		asIScriptModule* pModule;

		CleanupModuleOnExit( asIScriptModule* pModule )
			: pModule( pModule )
		{
		}

		~CleanupModuleOnExit()
		{
			if( pModule )
				pModule->Discard();
		}

		void Release()
		{
			pModule = nullptr;
		}
	} cleanupModule( pScriptModule );

	if( !builder.DefineWords( scriptBuilder ) )
	{
		return nullptr;
	}

	if( !builder.AddScripts( scriptBuilder ) )
	{
		return nullptr;
	}

	if( !builder.PreBuild( scriptBuilder ) )
	{
		return nullptr;
	}

	CASModule* pModule = nullptr;

	const bool bSuccess = scriptBuilder.BuildModule() >= 0;

	if( bSuccess )
	{
		pModule = new CASModule( scriptBuilder.GetModule() );
		cleanupModule.Release();
	}

	//Don't enter this if statement if bSuccess is false, that gets handled right after.
	if( !builder.PostBuild( scriptBuilder, bSuccess, pModule ) && bSuccess )
	{
		delete pModule;
		return nullptr;
	}

	if( !bSuccess )
	{
		return nullptr;
	}

	if( !AddModule( pModule ) )
	{
		delete pModule;
		return nullptr;
	}

	//This manager now holds a reference to the module. PostBuild may have added more references.
	pModule->Release();

	return pModule;
}

size_t CASModuleManager::GetModuleCount() const
{
	return m_Modules.size();
}

const CASModule* CASModuleManager::FindModuleByName( const char* const pszModuleName ) const
{
	assert( pszModuleName );

	if( !pszModuleName )
		return nullptr;

	auto it = std::find_if( m_Modules.begin(), m_Modules.end(), ModuleEqualByName( pszModuleName ) );

	return it != m_Modules.end() ? *it : nullptr;
}

CASModule* CASModuleManager::FindModuleByName( const char* const pszModuleName )
{
	return const_cast<CASModule*>( const_cast<const CASModuleManager*>( this )->FindModuleByName( pszModuleName ) );
}

const CASModule* CASModuleManager::FindModuleByIndex( const size_t uiIndex ) const
{
	assert( uiIndex < m_Modules.size() );

	return m_Modules[ uiIndex ];
}

CASModule* CASModuleManager::FindModuleByIndex( const size_t uiIndex )
{
	return const_cast<CASModule*>( const_cast<const CASModuleManager*>( this )->FindModuleByIndex( uiIndex ) );
}

bool CASModuleManager::AddModule( CASModule* pModule )
{
	assert( pModule );

	if( !pModule )
		return false;

	if( FindModuleByName( pModule->GetModuleName() ) )
		return false;

	pModule->AddRef();

	m_Modules.push_back( pModule );

	std::stable_sort( m_Modules.begin(), m_Modules.end(), ModuleLess );

	return true;
}

void CASModuleManager::RemoveModule( CASModule* pModule )
{
	if( !pModule )
		return;

	auto it = std::find( m_Modules.begin(), m_Modules.end(), pModule );

	if( it == m_Modules.end() )
		return;

	( *it )->Discard();
	( *it )->Release();

	m_Modules.erase( it );
}

void CASModuleManager::RemoveModule( const char* const pszModuleName )
{
	assert( pszModuleName );

	if( !pszModuleName )
		return;

	auto it = std::find_if( m_Modules.begin(), m_Modules.end(), ModuleEqualByName( pszModuleName ) );

	if( it == m_Modules.end() )
		return;

	( *it )->Discard();
	( *it )->Release();

	m_Modules.erase( it );
}

void CASModuleManager::Clear()
{
	for( auto pModule : m_Modules )
	{
		pModule->Discard();
		pModule->Release();
	}

	m_Modules.clear();
}
