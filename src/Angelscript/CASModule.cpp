#include <cassert>

#include <angelscript.h>

#include "CASModule.h"

CASModule::CASModule( asIScriptModule* pModule, const CASModuleDescriptor& descriptor )
	: m_pModule( pModule )
	, m_pDescriptor( &descriptor )
{
	assert( pModule );

	pModule->SetUserData( this, CASMODULE_USER_DATA_ID );
}

CASModule::~CASModule()
{
	Discard();
}

void CASModule::Release() const
{
	if( InternalRelease() )
	{
		delete this;
	}
}

void CASModule::Discard()
{
	if( m_pModule )
	{
		m_pModule->Discard();
		m_pModule = nullptr;
	}
}

const char* CASModule::GetModuleName() const
{
	assert( m_pModule );

	return m_pModule->GetName();
}

CASModule* GetModuleFromScriptModule( const asIScriptModule* pModule )
{
	assert( pModule );

	return reinterpret_cast<CASModule*>( pModule->GetUserData( CASMODULE_USER_DATA_ID ) );
}

CASModule* GetModuleFromScriptFunction( const asIScriptFunction* pFunction )
{
	assert( pFunction );

	auto pDelegate = pFunction->GetDelegateFunction();

	auto pFunc = pDelegate ? pDelegate : pFunction;

	auto pModule = GetModuleFromScriptModule( pFunc->GetModule() );

	return pModule;
}