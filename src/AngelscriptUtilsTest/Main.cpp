#include <iostream>

#include <angelscript.h>

#include "AngelscriptUtils/CASManager.h"
#include "AngelscriptUtils/event/CASEvent.h"
#include "AngelscriptUtils/event/CASEventCaller.h"
#include "AngelscriptUtils/CASModule.h"
#include "AngelscriptUtils/CASLoggingContextResultHandler.h"
#include "AngelscriptUtils/IASInitializer.h"
#include "AngelscriptUtils/IASModuleBuilder.h"

#include "AngelscriptUtils/add_on/scriptbuilder.h"
#include "AngelscriptUtils/add_on/scriptstdstring.h"
#include "AngelscriptUtils/add_on/scriptarray.h"
#include "AngelscriptUtils/add_on/scriptdictionary.h"
#include "AngelscriptUtils/add_on/scriptany.h"

#include "AngelscriptUtils/ScriptAPI/CASScheduler.h"
#include "AngelscriptUtils/ScriptAPI/Reflection/ASReflection.h"

#include "AngelscriptUtils/util/CASBaseClass.h"
#include "AngelscriptUtils/util/ASExtendAdapter.h"
#include "AngelscriptUtils/util/ASLogging.h"
#include "AngelscriptUtils/util/ASUtil.h"
#include "AngelscriptUtils/util/CASExtendAdapter.h"
#include "AngelscriptUtils/util/CASFileLogger.h"
#include "AngelscriptUtils/util/CASRefPtr.h"
#include "AngelscriptUtils/util/CASObjPtr.h"

#include "AngelscriptUtils/wrapper/ASCallable.h"
#include "AngelscriptUtils/wrapper/CASContext.h"

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
	NONE			= 0,

	/**
	*	Shared API.
	*/
	SHARED			= 1 << 0,

	/**
	*	Map script specific.
	*/
	MAPSCRIPT_ONLY	= 1 << 1,

	MAPSCRIPT		= SHARED | MAPSCRIPT_ONLY,

	/**
	*	Plugin script specific.
	*/
	PLUGIN_ONLY		= 1 << 2,

	PLUGIN			= SHARED | PLUGIN_ONLY,

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

asIScriptContext* CreateScriptContext( asIScriptEngine* pEngine, void* )
{
	auto pContext = pEngine->CreateContext();

	//TODO: add test to see if suspending will log an error.
	auto pResultHandler = new CASLoggingContextResultHandler( CASLoggingContextResultHandler::Flag::SUSPEND_IS_ERROR );

	as::SetContextResultHandler( *pContext, pResultHandler );

	pResultHandler->Release();

	return pContext;
}

void DestroyScriptContext( asIScriptEngine* ASUNREFERENCED( pEngine ), asIScriptContext* pContext, void* )
{
	if( pContext )
		pContext->Release();
}

const bool USE_EVENT_MANAGER = true;

/*
*	An event to test out the event system.
*	Stops as soon as it's handled.
*	Can be hooked by calling Events::Main.Hook( @MainHook( ... ) );
*/
CASEvent testEvent( "Main", "const " AS_STRING_OBJNAME "& in", "", ModuleAccessMask::ALL, EventStopMode::ON_HANDLED );

class CASTestInitializer : public IASInitializer
{
public:
	CASTestInitializer( CASManager& manager )
		: m_Manager( manager )
	{
	}

	bool UseEventManager() override { return USE_EVENT_MANAGER; }

	void OnInitBegin()
	{
		m_Manager.GetEngine()->SetContextCallbacks( &::CreateScriptContext, &::DestroyScriptContext );
	}

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

	bool AddEvents( CASManager& ASUNREFERENCED( manager ), CASEventManager& eventManager ) override
	{
		//Add an event. Scripts will be able to hook these, when it's invoked by C++ code all hooked functions are called.
		eventManager.AddEvent( &testEvent );

		return true;
	}

	bool RegisterAPI( CASManager& manager ) override
	{
		auto pEngine = manager.GetEngine();

		//Printing function.
		pEngine->RegisterGlobalFunction( "void Print(const " AS_STRING_OBJNAME "& in szString)", asFUNCTION( Print ), asCALL_CDECL );

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

private:
	CASManager& m_Manager;
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

		return builder.AddSectionFromFile( "resources/scripts/test.as" ) >= 0;
	}

	bool PostBuild( CScriptBuilder& ASUNREFERENCED( builder ), const bool bSuccess, CASModule* pModule ) override
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

/**
*	Logger that logs to a file and the console.
*/
class CASLogger : public CASBaseLogger<IASLogger>
{
public:
	CASLogger( const char* pszFilename, const CASFileLogger::Flags_t flags = CASFileLogger::Flag::NONE )
		: m_FileLogger( pszFilename, flags )
	{
	}

	void AddRef() const override
	{
		//Do nothing
	}

	void Release() const override
	{
		//Do nothing
	}

	void VLog( LogLevel_t logLevel, const char* pszFormat, va_list list ) override
	{
		m_FileLogger.VLog( logLevel, pszFormat, list );

		char szBuffer[ 4096 ];

		const int iResult = vsnprintf( szBuffer, sizeof( szBuffer ), pszFormat, list );

		if( iResult < 0 || static_cast<size_t>( iResult ) >= sizeof( szBuffer ) )
			return;

		std::cout << szBuffer;
	}

	//No need to call Release on this because it'll destruct in this class's destructor.
	CASFileLogger m_FileLogger;
};

CASLogger g_Logger( "logs/L", CASFileLogger::Flag::USE_DATESTAMP | CASFileLogger::Flag::USE_TIMESTAMP | CASFileLogger::Flag::OUTPUT_LOG_LEVEL );

int main( int ASUNREFERENCED( iArgc ), char* ASUNREFERENCED( pszArgV )[] )
{
	as::SetLogger( &g_Logger );

	std::cout << "Hello World!" << std::endl;

	CASManager manager;

	CASTestInitializer initializer( manager );

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
				as::Call( pFunction, &szString, false );

				if( USE_EVENT_MANAGER )
				{
					//Add main as a hook.
					testEvent.AddFunction( pFunction );

					//Trigger the event.
					CASEventCaller caller;

					caller.Call( testEvent, pEngine, &szString, true );
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

					std::cout << "Smart pointer points to: 0x" << std::hex << reinterpret_cast<intptr_t>( pPtr ) << std::dec << std::endl;
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

			//Call a function that triggers a null pointer exception.
			if( auto pFunction = pModule->GetModule()->GetFunctionByName( "DoNullPointerException" ) )
			{
				std::cout << "Triggering null pointer exception" << std::endl;
				as::Call( pFunction );
			}

			if( auto pFunction = pModule->GetModule()->GetFunctionByName( "DoNullPointerException2" ) )
			{
				std::cout << "Triggering null pointer exception in object member function" << std::endl;
				as::Call( pFunction );
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

				int result = pBaseEnt->ScheduleOfType( "foo" );

				//Silence compiler warnings (unused var).
				result = result;

				delete pEntity;
			}

			std::cout << "Created extend class: " << ( bCreatedExtend ? "yes" : "no" ) << std::endl;

			manager.GetEventManager()->DumpHookedFunctions();

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