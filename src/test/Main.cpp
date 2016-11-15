#include <iostream>

#include <angelscript.h>

#include "Angelscript/CASManager.h"
#include "Angelscript/CASEvent.h"
#include "Angelscript/CASModule.h"
#include "Angelscript/IASInitializer.h"
#include "Angelscript/IASModuleBuilder.h"

#include "Angelscript/add_on/scriptbuilder.h"
#include "Angelscript/add_on/scriptstdstring.h"
#include "Angelscript/add_on/scriptarray.h"
#include "Angelscript/add_on/scriptdictionary.h"
#include "Angelscript/add_on/scriptany.h"

#include "Angelscript/ScriptAPI/CASScheduler.h"
#include "Angelscript/ScriptAPI/Reflection/ASReflection.h"

#include "Angelscript/util/ASExtendAdapter.h"
#include "Angelscript/util/ASUtil.h"
#include "Angelscript/util/CASExtendAdapter.h"
#include "Angelscript/util/CASRefPtr.h"
#include "Angelscript/util/CASObjPtr.h"

#include "Angelscript/wrapper/ASCallable.h"
#include "Angelscript/wrapper/CASContext.h"

#include "CBaseEntity.h"
#include "CScriptBaseEntity.h"
#include "ASCBaseEntity.h"

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
	NONE		= 0,

	/**
	*	Shared API.
	*/
	SHARED		= 1 << 0,

	/**
	*	Map script specific.
	*/
	MAPSCRIPT	= SHARED | 1 << 1,

	/**
	*	Plugin script specific.
	*/
	PLUGIN		= SHARED | 1 << 2,

	/**
	*	All scripts.
	*/
	ALL			= SHARED | MAPSCRIPT | PLUGIN
};
}

void Print( const std::string& szString )
{
	std::cout << szString;
}

int NSTest()
{
	return 0;
}

const bool USE_EVENT_MANAGER = true;

/*
*	An event to test out the event system.
*	Stops as soon as it's handled.
*	Can be hooked by calling Events::Main.Hook( @MainHook( ... ) );
*/
CASEvent event( "Main", "const string& in", "", ModuleAccessMask::ALL, EventStopMode::ON_HANDLED );

class CASTestInitializer : public IASInitializer
{
public:
	bool UseEventManager() override { return USE_EVENT_MANAGER; }

	bool RegisterCoreAPI( CASManager& manager ) override
	{
		RegisterStdString( manager.GetEngine() );
		RegisterScriptArray( manager.GetEngine(), true );
		RegisterScriptDictionary( manager.GetEngine() );
		RegisterScriptAny( manager.GetEngine() );
		RegisterScriptScheduler( manager.GetEngine() );
		RegisterScriptReflection( *manager.GetEngine() );

		RegisterScriptEventAPI( *manager.GetEngine() );

		manager.GetEngine()->RegisterTypedef( "size_t", "uint32" );

		return true;
	}

	bool AddEvents( CASManager& manager, CASEventManager& eventManager ) override
	{
		//Add an event. Scripts will be able to hook these, when it's invoked by C++ code all hooked functions are called.
		eventManager.AddEvent( &event );

		return true;
	}

	bool RegisterAPI( CASManager& manager ) override
	{
		auto pEngine = manager.GetEngine();

		//Printing function.
		pEngine->RegisterGlobalFunction( "void Print(const string& in szString)", asFUNCTION( Print ), asCALL_CDECL );

		pEngine->SetDefaultNamespace( "NS" );

		pEngine->RegisterGlobalFunction( 
			"int NSTest()", 
			asFUNCTION( NSTest ),
			asCALL_CDECL );

		pEngine->SetDefaultNamespace( "" );

		//Register the interface that all custom entities use. Allows you to take them as handles to functions.
		pEngine->RegisterInterface( "IScriptEntity" );

		//Register the entity class.
		RegisterScriptCBaseEntity( *pEngine );

		//Register the entity base class. Used to call base class implementations.
		RegisterScriptBaseEntity( *pEngine );

		return true;
	}
};

/**
*	Builder for the test script.
*/
class CASTestModuleBuilder : public IASModuleBuilder
{
public:
	CASTestModuleBuilder( const std::string& szDecl )
		: m_szDecl( szDecl )
	{
	}

	bool AddScripts( CScriptBuilder& builder ) override
	{
		//By using a handle this can be changed, but since there are no other instances, it can only be made null.
		//TODO: figure out a better way.
		auto result = builder.AddSectionFromMemory( 
			"__Globals", 
			"CScheduler@ Scheduler;" );

		if( result < 0 )
			return false;

		if( builder.AddSectionFromMemory(
			"__CScriptBaseEntity",
			m_szDecl.c_str() ) < 0 )
			return false;

		return builder.AddSectionFromFile( "scripts/test.as" ) >= 0;
	}

	bool PostBuild( CScriptBuilder& builder, const bool bSuccess, CASModule* pModule ) override
	{
		if( !bSuccess )
			return false;

		auto& scriptModule = *pModule->GetModule();

		//Set the scheduler instance.
		if( !as::SetGlobalByName( scriptModule, "Scheduler", pModule->GetScheduler() ) )
			return false;

		return true;
	}

private:
	std::string m_szDecl;
};

class CASModuleUserData : public IASModuleUserData
{
public:
	CASModuleUserData()
	{
		std::cout << "Creating user data" << std::endl;
	}

	~CASModuleUserData()
	{
		std::cout << "Destroying user data" << std::endl;
	}

	void Release() const override
	{
		delete this;
	}
};

int main( int iArgc, char* pszArgV[] )
{
	std::cout << "Hello World!" << std::endl;

	CASManager manager;

	CASTestInitializer initializer;

	if( manager.Initialize( initializer ) )
	{
		auto pEngine = manager.GetEngine();

		//Create the declaration used for script entity base classes.
		const auto szDecl = as::CreateExtendBaseclassDeclaration( "CScriptBaseEntity", "IScriptEntity", "CBaseEntity", "BaseEntity" );

		std::cout << szDecl << std::endl;

		//Create some module types.

		//Map scripts are per-map scripts that always have their hooks executed before any other module.
		manager.GetModuleManager().AddDescriptor( "MapScript", ModuleAccessMask::MAPSCRIPT, as::ModulePriority::HIGHEST );

		//Plugins are persistent scripts that can keep running after map changes.
		manager.GetModuleManager().AddDescriptor( "Plugin", ModuleAccessMask::PLUGIN );

		//Make a map script.
		CASTestModuleBuilder builder( szDecl );

		auto pModule = manager.GetModuleManager().BuildModule( "MapScript", "MapModule", builder, new CASModuleUserData() );

		if( pModule )
		{
			//Call the main function.
			if( auto pFunction = pModule->GetModule()->GetFunctionByName( "main" ) )
			{
				std::string szString = "Hello World!\n";

				//Note: main takes a const string& in, so pass the address here. References are handled as pointers.
				as::Call( pFunction, &szString );

				if( USE_EVENT_MANAGER )
				{
					//Add main as a hook.
					event.AddFunction( pFunction );

					//Trigger the event.
					event.Call( CallFlag::NONE, &szString );
				}
			}

			//Test the object pointer.
			if( auto pFunction = pModule->GetModule()->GetFunctionByName( "GetLifetime" ) )
			{
				CASOwningContext ctx( *manager.GetEngine() );

				CASFunction func( *pFunction, ctx );

				if( func.Call( CallFlag::NONE ) )
				{
					CASObjPtr ptr;

					void* pThis;

					if( func.GetReturnValue( &pThis ) )
					{
						const int iTypeId = pFunction->GetReturnTypeId();

						ptr.Set( pThis, manager.GetEngine()->GetTypeInfoById( iTypeId ) );
					}

					if( ptr )
					{
						std::cout << "Object stored" << std::endl << "Type: " << ptr.GetTypeInfo()->GetName() << std::endl;
					}
					else
					{
						std::cout << "Object not stored" << std::endl;
					}
				}
			}

			//Call a function using the different function call helpers.
			if( auto pFunction = pModule->GetModule()->GetFunctionByName( "NoArgs" ) )
			{
				{
					//Test the smart pointer.
					CASRefPtr<asIScriptFunction> func;

					CASRefPtr<asIScriptFunction> func2( pFunction );

					func = func2;

					func = std::move( func2 );

					CASRefPtr<asIScriptFunction> func3( func );

					CASRefPtr<asIScriptFunction> func4( std::move( func ) );

					func.Set( pFunction );

					auto pPtr = func.Get();
				}

				//Regular varargs.
				as::Call( pFunction );
				//Argument list.
				as::CallArgs( pFunction, CASArguments() );

				struct Helper final
				{
				public:
					Helper( asIScriptFunction* pFunction )
						: pFunction( pFunction )
					{
					}

					void Call( CallFlags_t flags, ... )
					{
						va_list list;

						va_start( list, flags );

						as::VCall( flags, pFunction, list );

						va_end( list );
					}

				private:
					asIScriptFunction* pFunction;
				};

				//va_list version.
				Helper helper( pFunction );
				helper.Call( CallFlag::NONE );
			}

			//Test the scheduler.
			pModule->GetScheduler()->Think( 10 );

			//Get the parameter types. Angelscript's type info support isn't complete yet, so not all types have an asITypeInfo instance yet.
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

			//Try to create a C++ class that is extended in a script.
			bool bCreatedExtend = false;

			//This data will need to be stored somewhere, bound to the baseclass.
			if( auto pEntity = as::CreateExtensionClassInstance<CScriptBaseEntity>( *pEngine, *pModule->GetModule(), "CEntity", "CBaseEntity", "BaseEntity" ) )
			{
				bCreatedExtend = true;

				CBaseEntity* pBaseEnt = pEntity;

				pBaseEnt->Spawn();

				const int result = pBaseEnt->ScheduleOfType( "foo" );

				int x = 10;

				delete pEntity;
			}

			std::cout << "Created extend class: " << ( bCreatedExtend ? "yes" : "no" ) << std::endl;

			//Remove the module.
			manager.GetModuleManager().RemoveModule( pModule );
		}
	}

	//Shut down the Angelscript engine, frees all resources.
	manager.Shutdown();

	//Wait for input.
	getchar();

	return 0;
}