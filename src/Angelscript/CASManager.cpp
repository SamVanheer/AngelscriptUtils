#include <cassert>
#include <iostream>

#include <angelscript.h>

#include "IASInitializer.h"

#include "CASManager.h"

CASManager* CASManager::m_pActiveManager = nullptr;

CASManager* CASManager::GetActiveManager()
{
	assert( m_pActiveManager );

	return m_pActiveManager;
}

void CASManager::ActivateManager( CASManager* pManager )
{
	m_pActiveManager = pManager;
}

void CASManager::Activate()
{
	ActivateManager( this );
}

void CASManager::Deactivate()
{
	if( this == m_pActiveManager )
	{
		m_pActiveManager = nullptr;
	}
}

CASManager::CASManager()
	: m_ModuleManager( *this )
{
}

CASManager::~CASManager()
{
	assert( !m_pScriptEngine );
}

bool CASManager::Initialize( IASInitializer& initializer )
{
	if( m_pScriptEngine )
	{
		return true;
	}

	m_pScriptEngine = asCreateScriptEngine( ANGELSCRIPT_VERSION );

	if( !m_pScriptEngine )
	{
		return false;
	}

	m_EventManager = std::make_unique<CASEventManager>( *m_pScriptEngine, initializer.GetEventNamespace() );

	asSFuncPtr msgCallback;
	void* pObj;
	asDWORD callConv;

	if( !initializer.GetMessageCallback( msgCallback, pObj, callConv ) )
	{
		msgCallback = asMETHOD( CASManager, MessageCallback );
		pObj = this;
		callConv = asCALL_THISCALL;
	}

	m_pScriptEngine->SetMessageCallback( msgCallback, pObj, callConv );

	Activate();

	if( !initializer.RegisterCoreAPI( *this ) )
		return false;

	if( !initializer.AddEvents( *this, *m_EventManager ) )
		return false;

	//Registers all events. One-time event that happens on startup.
	m_EventManager->RegisterEvents( *GetEngine() );

	if( !initializer.RegisterAPI( *this ) )
		return false;

	return true;
}

void CASManager::Shutdown()
{
	if( !m_pScriptEngine )
	{
		return;
	}

	Activate();

	if( m_EventManager )
	{
		//Unhook all functions to prevent dangling pointers.
		m_EventManager->UnhookAllFunctions();

		m_EventManager.reset();
	}

	//Clear all modules and destroy all descriptors.
	m_ModuleManager.Clear();

	//Let it go.
	m_pScriptEngine->ShutDownAndRelease();
	m_pScriptEngine = nullptr;

	Deactivate();
}

void CASManager::MessageCallback( const asSMessageInfo* pMsg )
{
	const char* pType = "";

	//Get the prefix.
	switch( pMsg->type )
	{
	case asMSGTYPE_ERROR: pType = "Error: "; break;
	case asMSGTYPE_WARNING: pType = "Warning: "; break;
	default: break;
	}

	//Only display the section if it was actually set. Some messages are not triggered by script code compilation or execution.
	const bool bHasSection = pMsg->section && *pMsg->section;

	bool bNeedsNewline = false;

	if( bHasSection )
	{
		std::cout << "Section \"" << pMsg->section << "\"";
		bNeedsNewline = true;
	}

	//Some messages don't refer to script code, and set both to 0.
	if( pMsg->row != 0 && pMsg->col != 0 )
	{
		if( bHasSection )
			std::cout << ' ';

		std::cout << "(" << pMsg->row << ", " << pMsg->col << ")";
		bNeedsNewline = true;
	}

	if( bNeedsNewline )
		std::cout << std::endl;

	std::cout << pType << pMsg->message << std::endl;
}