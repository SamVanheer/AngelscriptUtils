#include <iostream>

#include <angelscript.h>

#include "Angelscript/add_on/scriptbuilder.h"
#include "Angelscript/add_on/scriptstdstring.h"

#include "wrapper/ASCallable.h"
#include "wrapper/CASContext.h"

class CASManager final
{
public:
	CASManager() = default;
	~CASManager() = default;

	asIScriptEngine* GetEngine() { return m_pScriptEngine; }

	bool Initialize();

	void Shutdown();

private:
	void MessageCallback( const asSMessageInfo* pMsg );

private:
	asIScriptEngine* m_pScriptEngine = nullptr;

private:
	CASManager( const CASManager& ) = delete;
	CASManager& operator=( const CASManager& ) = delete;
};

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

void Print( const std::string& szString )
{
	std::cout << szString;
}

int main( int iArgc, char* pszArgV[] )
{
	std::cout << "Hello World!" << std::endl;

	CASManager manager;

	if( manager.Initialize() )
	{
		RegisterStdString( manager.GetEngine() );

		manager.GetEngine()->RegisterGlobalFunction( "void Print( const string& in szString)", asFUNCTION( Print ), asCALL_CDECL );

		CScriptBuilder builder;

		builder.StartNewModule( manager.GetEngine(), "Test" );

		builder.AddSectionFromFile( "external/test.as" );

		builder.BuildModule();

		auto pModule = builder.GetModule();

		auto pFunction = pModule->GetFunctionByName( "main" );

		as::CallFunction( pFunction );

		pFunction->Release();

		pModule->Discard();
	}

	manager.Shutdown();

	getchar();

	return 0;
}