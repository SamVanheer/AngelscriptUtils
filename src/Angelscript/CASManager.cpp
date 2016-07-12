#include <iostream>

#include <angelscript.h>

#include "CASManager.h"

CASManager::CASManager()
	: m_ModuleManager( *this )
	, m_HookManager( *this )
{
}

bool CASManager::Initialize()
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

	m_pScriptEngine->SetMessageCallback( asMETHOD( CASManager, MessageCallback ), this, asCALL_THISCALL );

	return true;
}

void CASManager::Shutdown()
{
	if( !m_pScriptEngine )
	{
		return;
	}

	m_ModuleManager.Clear();

	m_pScriptEngine->ShutDownAndRelease();
}

void CASManager::MessageCallback( const asSMessageInfo* pMsg )
{
	const char* pType = "";

	switch( pMsg->type )
	{
	case asMSGTYPE_ERROR: pType = "Error: "; break;
	case asMSGTYPE_WARNING: pType = "Warning: "; break;
	default: break;
	}

	const bool bHasSection = pMsg->section && *pMsg->section;

	bool bNeedsNewline = false;

	if( bHasSection )
	{
		std::cout << "Section \"" << pMsg->section << "\"";
		bNeedsNewline = true;
	}

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