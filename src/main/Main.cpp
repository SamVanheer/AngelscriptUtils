#include <iostream>

#include <angelscript.h>

#include "Angelscript/CASManager.h"
#include "Angelscript/CASHook.h"
#include "Angelscript/CASModule.h"
#include "Angelscript/IASModuleBuilder.h"

#include "Angelscript/add_on/scriptbuilder.h"
#include "Angelscript/add_on/scriptstdstring.h"
#include "Angelscript/add_on/scriptarray.h"
#include "Angelscript/add_on/scriptdictionary.h"
#include "Angelscript/add_on/scriptany.h"

#include "Angelscript/ScriptAPI/CASScheduler.h"

#include "Angelscript/util/ASUtil.h"

#include "Angelscript/wrapper/ASCallable.h"
#include "Angelscript/wrapper/CASContext.h"

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
		//By using a handle this can be changed, but since there are no other instances, it can only be made null.
		//TODO: figure out a better way.
		auto result = builder.AddSectionFromMemory( 
			"__Globals", 
			"CScheduler@ Scheduler;" );

		if( result < 0 )
			return false;

		return builder.AddSectionFromFile( "external/test.as" ) >= 0;
	}

	bool PostBuild( const bool bSuccess, CASModule* pModule )
	{
		if( !bSuccess )
			return false;

		auto& scriptModule = *pModule->GetModule();

		if( !as::SetGlobalByName( scriptModule, "Scheduler", pModule->GetScheduler() ) )
			return false;

		return true;
	}
};

namespace ModuleAccessMask
{
/**
*	Access masks for modules.
*/
enum ModuleAccessMask
{
	/**
	*	No access.
	*/
	NONE			= 0,

	/**
	*	Shared API.
	*/
	SHARED			= 1 << 0,

	/**
	*	Map script specific.
	*/
	MAPSCRIPT		= SHARED | 1 << 1,

	/**
	*	Plugin script specific.
	*/
	PLUGIN			= SHARED | 1 << 2,

	/**
	*	All scripts.
	*/
	ALL				= SHARED | MAPSCRIPT | PLUGIN
};
}

int main( int iArgc, char* pszArgV[] )
{
	std::cout << "Hello World!" << std::endl;

	CASManager manager;

	if( manager.Initialize() )
	{
		RegisterStdString( manager.GetEngine() );
		RegisterScriptArray( manager.GetEngine(), true );
		RegisterScriptDictionary( manager.GetEngine() );
		RegisterScriptAny( manager.GetEngine() );
		RegisterScriptScheduler( manager.GetEngine() );

		manager.GetEngine()->RegisterTypedef( "size_t", "uint32" );

		manager.GetHookManager().AddHook( &hook );

		manager.GetHookManager().RegisterHooks( *manager.GetEngine() );

		manager.GetEngine()->RegisterGlobalFunction( "void Print(const string& in szString)", asFUNCTION( Print ), asCALL_CDECL );

		//Map scripts are per-map scripts that always have their hooks executed before any other module.
		manager.GetModuleManager().AddDescriptor( "MapScript", ModuleAccessMask::MAPSCRIPT, as::ModulePriority::HIGHEST );

		//Plugins are persistent scripts that can keep running after map changes.
		manager.GetModuleManager().AddDescriptor( "Plugin", ModuleAccessMask::PLUGIN );

		CASGenericModuleBuilder builder;

		auto pModule = manager.GetModuleManager().BuildModule( "MapScript", "MapModule", builder );

		if( pModule )
		{
			if( auto pFunction = pModule->GetModule()->GetFunctionByName( "main" ) )
			{
				hook.AddFunction( pFunction );

				std::string szString = "Hello World!\n";

				as::CallFunction( pFunction, &szString );

				hook.Call( CallFlag::NONE, &szString );
			}

			pModule->GetScheduler()->Think( 10 );

			/*
			if( auto pFunc2 = pModule->GetModule()->GetFunctionByName( "Function" ) )
			{
				for( asUINT uiIndex = 0; uiIndex < pFunc2->GetParamCount(); ++uiIndex )
				{
					int iTypeId;
					const char* pszName;
					pFunc2->GetParam( uiIndex, &iTypeId, nullptr, &pszName );

					std::cout << "Parameter " << uiIndex << ": " << pszName << std::endl;
				
					if( auto pType = manager.GetEngine()->GetTypeInfoById( iTypeId ) )
					{
						std::cout << pType->GetNamespace() << "::" << pType->GetName() << std::endl;

						asDWORD uiFlags = pType->GetFlags();

						if( uiFlags & asOBJ_VALUE )
							std::cout << "Value" << std::endl;

						if( uiFlags & asOBJ_REF )
							std::cout << "Ref" << std::endl;

						if( uiFlags & asOBJ_ENUM )
							std::cout << "Enum" << std::endl;

						if( uiFlags & asOBJ_FUNCDEF )
							std::cout << "Funcdef" << std::endl;

						if( uiFlags & asOBJ_POD )
							std::cout << "POD" << std::endl;

						if( uiFlags & asOBJ_TYPEDEF )
							std::cout << "Typedef" << std::endl;
					}
					else //Only primitive types don't have type info right now.
						std::cout << "No type info" << std::endl;
				}
			}
			*/

			manager.GetHookManager().UnhookModuleFunctions( pModule );

			manager.GetModuleManager().RemoveModule( pModule );
		}
	}

	manager.Shutdown();

	getchar();

	return 0;
}