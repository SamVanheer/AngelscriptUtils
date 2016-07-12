#include <iostream>

#include <angelscript.h>

#include "Angelscript/add_on/scriptbuilder.h"
#include "Angelscript/add_on/scriptstdstring.h"
#include "Angelscript/add_on/scriptarray.h"
#include "Angelscript/add_on/scriptdictionary.h"

#include "Angelscript/wrapper/ASCallable.h"
#include "Angelscript/wrapper/CASContext.h"
#include "Angelscript/CASModule.h"
#include "Angelscript/CASHook.h"

#include "Angelscript/IASModuleBuilder.h"

#include "Angelscript/CASManager.h"

void Print( const std::string& szString )
{
	std::cout << szString;
}

CASHook hook( "Main", "const string& in", "", 0xFFF, HookStopMode::ON_HANDLED );

class CASGenericModuleBuilder : public IASModuleBuilder
{
public:

	bool AddScripts( CScriptBuilder& builder ) override
	{
		return builder.AddSectionFromFile( "external/test.as" ) >= 0;
	}
};

int main( int iArgc, char* pszArgV[] )
{
	std::cout << "Hello World!" << std::endl;

	CASManager manager;

	if( manager.Initialize() )
	{
		RegisterStdString( manager.GetEngine() );
		RegisterScriptArray( manager.GetEngine(), true );
		RegisterScriptDictionary( manager.GetEngine() );

		manager.GetHookManager().AddHook( &hook );

		manager.GetHookManager().RegisterHooks( *manager.GetEngine() );

		manager.GetEngine()->RegisterGlobalFunction( "void Print(const string& in szString)", asFUNCTION( Print ), asCALL_CDECL );

		//Map scripts are per-map scripts that always have their hooks executed before any other module.
		manager.GetModuleManager().AddDescriptor( "MapScript", 0xFFFF, as::ModulePriority::HIGHEST );

		//Plugins are persistent scripts that can keep running after map changes.
		manager.GetModuleManager().AddDescriptor( "Plugin", 0xFFFE );

		CASGenericModuleBuilder builder;

		auto pModule = manager.GetModuleManager().BuildModule( "MapScript", "MapModule", builder );

		if( pModule )
		{
			auto pFunction = pModule->GetModule()->GetFunctionByName( "main" );

			hook.AddFunction( pFunction );

			std::string szString = "Hello World!\n";

			as::CallFunction( pFunction, &szString );

			hook.Call( CallFlag::NONE, &szString );

			manager.GetHookManager().UnhookModuleFunctions( pModule );

			manager.GetModuleManager().RemoveModule( pModule );
		}
	}

	manager.Shutdown();

	getchar();

	return 0;
}