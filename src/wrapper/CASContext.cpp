#include <cassert>

#include "CASContext.h"

CASContext::CASContext( asIScriptContext& context )
{
	m_pContext = &context;

	m_pContext->AddRef();
}

CASContext::CASContext( asIScriptEngine& engine )
{
	m_pContext = engine.RequestContext();
	m_pEngine = &engine;

	m_pEngine->AddRef();
}

CASContext::~CASContext()
{
	Release();
}

void CASContext::Release()
{
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

CASNonOwningContext::~CASNonOwningContext()
{
	ReleaseOwnership();
}