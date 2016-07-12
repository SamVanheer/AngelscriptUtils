#include <cassert>

#include "CASContext.h"

CASContext::CASContext( asIScriptContext& context )
{
	m_pContext = &context;
}

void CASContext::Release()
{
	if( m_pContext )
	{
		//TODO return value
		m_pContext->Unprepare();
	}

	if( m_pEngine )
	{
		m_pEngine->ReturnContext( m_pContext );
		m_pContext = nullptr;

		m_pEngine->Release();
		m_pEngine = nullptr;
	}
	else if( m_pContext )
	{
		m_pContext->Release();
		m_pContext = nullptr;
	}
}

void CASContext::ReleaseOwnership()
{
	if( m_pContext )
	{
		m_pContext = nullptr;
	}

	if( m_pEngine )
	{
		m_pEngine = nullptr;
	}
}

CASContext::operator bool() const
{
	return m_pContext != nullptr;
}

CASOwningContext::CASOwningContext( asIScriptContext& context )
	: CASContext( context )
{
	m_pContext->AddRef();
}

CASOwningContext::CASOwningContext( asIScriptEngine& engine )
{
	m_pContext = engine.RequestContext();
	m_pEngine = &engine;

	m_pEngine->AddRef();
}

CASOwningContext::~CASOwningContext()
{
	Release();
}