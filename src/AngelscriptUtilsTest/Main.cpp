#include <iostream>
#include <vector>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/daily_file_sink.h>

#include <angelscript.h>

#undef VOID

#include "AngelscriptUtils/CASManager.h"
#include "AngelscriptUtils/CASModule.h"
#include "AngelscriptUtils/IASInitializer.h"
#include "AngelscriptUtils/IASModuleBuilder.h"

#include "AngelscriptUtils/add_on/scriptbuilder/scriptbuilder.h"

#include "AngelscriptUtils/ScriptAPI/Scheduler.h"
#include "AngelscriptUtils/ScriptAPI/Reflection/ASReflection.h"

#include "AngelscriptUtils/util/CASBaseClass.h"
#include "AngelscriptUtils/util/ASExtendAdapter.h"
#include "AngelscriptUtils/util/ASLogging.h"
#include "AngelscriptUtils/util/CASExtendAdapter.h"

#include "AngelscriptUtils/utility/Objects.h"
#include "AngelscriptUtils/utility/SmartPointers.h"
#include "AngelscriptUtils/utility/Variant.h"

#include "AngelscriptUtils/wrapper/ASCallable.h"
#include "AngelscriptUtils/wrapper/CASContext.h"
#include "AngelscriptUtils/wrapper/WrappedScriptContext.h"

#include "add_on/scriptany/scriptany.h"
#include "add_on/scriptarray/scriptarray.h"
#include "add_on/scriptdictionary/scriptdictionary.h"
#include "add_on/scriptstdstring/scriptstdstring.h"

#include "CBaseEntity.h"
#include "CScriptBaseEntity.h"
#include "ASCBaseEntity.h"

#include "AngelscriptUtils/event/EventArgs.h"
#include "AngelscriptUtils/event/EventScriptAPI.h"
#include "AngelscriptUtils/event/EventSystem.h"

#include "AngelscriptUtils/execution/Metadata.h"
#include "AngelscriptUtils/execution/Execution.h"
#include "AngelscriptUtils/execution/Packing.h"

DEFINE_OBJECT_TYPE_SIMPLE(std::string, string, asOBJ_VALUE | asGetTypeTraits<std::string>())

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
	auto context = pEngine->CreateContext();

	//TODO: add test to see if suspending will log an error.
	auto wrapper = new asutils::LoggingScriptContext(*context, as::log, true);

	context->Release();

	return wrapper;
}

void DestroyScriptContext( asIScriptEngine*, asIScriptContext* pContext, void* )
{
	//Releases both the wrapper and context at once if this is a wrapped context
	if( pContext )
		pContext->Release();
}

class MyEvent : public asutils::EventArgs
{
public:
	MyEvent() = default;

	MyEvent(const MyEvent&) = default;

	bool ShouldHide() const { return m_ShouldHide; }

	void SetShouldHide(bool value)
	{
		m_ShouldHide = value;
	}

private:
	bool m_ShouldHide = false;
};

DEFINE_OBJECT_TYPE_SIMPLE(MyEvent, MyEvent, asOBJ_REF | asGetTypeTraits<MyEvent>())

enum class EnumType
{
	Value = 1
};

DEFINE_ENUM_TYPE_SIMPLE(EnumType, EnumType)

void RegisterMyEvent(asIScriptEngine& engine)
{
	const auto className = "MyEvent";

	asutils::RegisterEventClass<MyEvent>(engine, className);

	engine.RegisterObjectMethod(className, "bool get_ShouldHide() const property", asMETHOD(MyEvent, ShouldHide), asCALL_THISCALL);
	engine.RegisterObjectMethod(className, "void set_ShouldHide(bool value) property", asMETHOD(MyEvent, SetShouldHide), asCALL_THISCALL);
}

class CASTestInitializer : public IASInitializer
{
public:
	CASTestInitializer( CASManager& manager, asutils::EventRegistry& eventRegistry)
		: m_Manager( manager )
		, m_EventRegistry(eventRegistry)
	{
	}

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
		asutils::RegisterSchedulerAPI( *manager.GetEngine() );
		RegisterScriptReflection( *manager.GetEngine() );

		asutils::RegisterEventAPI(*manager.GetEngine());

		RegisterMyEvent(*manager.GetEngine());

		m_EventSystem = std::make_unique<asutils::EventSystem>(m_EventRegistry, asutils::ReferencePointer<asIScriptContext>(manager.GetEngine()->RequestContext(), true));

		manager.GetEngine()->RegisterGlobalProperty("EventSystem g_EventSystem", m_EventSystem.get());

		manager.GetEngine()->RegisterTypedef( "size_t", "uint32" );

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

	asutils::EventSystem& GetEventSystem()
	{
		return *m_EventSystem;
	}

private:
	CASManager& m_Manager;
	asutils::EventRegistry& m_EventRegistry;

	std::unique_ptr<asutils::EventSystem> m_EventSystem;

private:
	CASTestInitializer( const CASTestInitializer& ) = delete;
	CASTestInitializer& operator=( const CASTestInitializer& ) = delete;
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
			"ScriptScheduler@ Scheduler;" );

		if( result < 0 )
			return false;

		if( builder.AddSectionFromMemory(
			"__CScriptBaseEntity",
			m_szDecl.c_str() ) < 0 )
			return false;

		return builder.AddSectionFromFile( "resources/scripts/test.as" ) >= 0;
	}

	bool PostBuild( CScriptBuilder&, const bool bSuccess, CASModule* pModule ) override
	{
		if( !bSuccess )
			return false;

		auto& scriptModule = *pModule->GetModule();

		//Set the scheduler instance.
		if( !asutils::SetGlobalByName( scriptModule, "Scheduler", &pModule->GetScheduler() ) )
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

struct CallHelper final
{
public:
	CallHelper(asIScriptFunction& function, asIScriptContext& context)
		: function(function)
		, context(context)
	{
	}

	template<typename... PARAMS>
	void Call(PARAMS&&... params)
	{
		asutils::NativeCall(function, context, std::forward<PARAMS>(params)...);
	}

private:
	asIScriptFunction& function;
	asIScriptContext& context;
};

int main( int, char*[] )
{
	{
		auto console = std::make_shared<spdlog::sinks::stdout_sink_mt>();
		auto file = std::make_shared<spdlog::sinks::daily_file_sink_mt>( "logs/L", 0, 0 );

		auto logger = std::make_shared<spdlog::logger>( "ASUtils", spdlog::sinks_init_list{ console, file } );

		as::log = logger;
	}

	std::cout << "Hello World!" << std::endl;

	CASManager manager;

	asutils::EventRegistry registry;

	CASTestInitializer initializer( manager, registry);

	if( manager.Initialize( initializer ) )
	{
		auto pEngine = manager.GetEngine();

		asutils::Variant variant(10);

		variant.Reset(10);

		registry.Register<MyEvent>(*pEngine->GetTypeInfoByName("MyEvent"));

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
			auto pContext = pEngine->RequestContext();

			auto& eventSystem = initializer.GetEventSystem();

			if (auto pFunction = pModule->GetModule()->GetFunctionByName("TemplatedCallTest"))
			{
				auto parameters = asutils::CreateNativeParameterList(10, EnumType::Value, new MyEvent(), std::string{"Packed parameters"});

				asutils::PackedCall(*pFunction, *pContext, parameters);

				int output = 10;
				auto event = new MyEvent();
				event->AddRef();
				asutils::NativeCall(*pFunction, *pContext, output, EnumType::Value, event, std::string{"Test string"});

				std::cout << "C++ value of foo is " << output << std::endl;
			}

			//Call the main function.
			if( auto pFunction = pModule->GetModule()->GetFunctionByName( "main" ) )
			{
				std::string szString = "Hello World!\n";

				//Note: main takes a const string& in. We can pass by pointer or by reference, either will work the same way
				//Constructing the string in the parameter list itself also works
				asutils::NativeCall( *pFunction, *pContext, &szString );

				MyEvent event;

				eventSystem.GetEvent<MyEvent>().Dispatch(event);

				std::cout << "Should hide: " << event.ShouldHide() << std::endl;
			}

			//Test the object pointer.
			if( auto pFunction = pModule->GetModule()->GetFunctionByName( "GetLifetime" ) )
			{
				{
					asutils::FunctionExecutor executor(*pContext);

					auto functionCaller = executor.Global(*pFunction);

					if (functionCaller.NativeCall())
					{
						asutils::ObjectPointer ptr;

						auto result = functionCaller.GetObjectReturnValue();

						if (result)
						{
							const int iTypeId = pFunction->GetReturnTypeId();

							ptr = result;
						}

						if (ptr)
						{
							std::cout << "Object stored" << std::endl << "Type: " << ptr.GetTypeInfo()->GetName() << std::endl;

							auto memberFunction = ptr.GetTypeInfo()->GetMethodByName("SayHi");

							if (memberFunction)
							{
								auto memberCaller = executor.Member(*memberFunction);

								memberCaller.NativeCall(ptr.Get());
							}
							else
							{
								std::cout << "Couldn't get SayHi method" << std::endl;
							}
						}
						else
						{
							std::cout << "Object not stored" << std::endl;
						}
					}
				}
			}

			//Call a function using the different function call helpers.
			if( auto pFunction = pModule->GetModule()->GetFunctionByName( "NoArgs" ) )
			{
				{
					//Test the smart pointer.
					asutils::ReferencePointer<asIScriptFunction> func;

					asutils::ReferencePointer<asIScriptFunction> func2( pFunction );

					func = func2;

					func = std::move( func2 );

					asutils::ReferencePointer<asIScriptFunction> func3( func );

					asutils::ReferencePointer<asIScriptFunction> func4( std::move( func ) );

					func.Reset( pFunction );

					auto pPtr = func.Get();

					std::cout << "Smart pointer points to: 0x" << std::hex << reinterpret_cast<intptr_t>( pPtr ) << std::dec << std::endl;
				}

				//Regular variadic templates
				asutils::NativeCall(*pFunction, *pContext);

				//Argument list
				auto parameterList = asutils::CreateNativeParameterList();
				asutils::PackedCall(*pFunction, *pContext, parameterList);

				//Wrapper version
				CallHelper helper(*pFunction, *pContext);
				helper.Call();
			}

			//Call a function that triggers a null pointer exception.
			if( auto pFunction = pModule->GetModule()->GetFunctionByName( "DoNullPointerException" ) )
			{
				std::cout << "Triggering null pointer exception" << std::endl;
				asutils::NativeCall( *pFunction, *pContext );
			}

			if( auto pFunction = pModule->GetModule()->GetFunctionByName( "DoNullPointerException2" ) )
			{
				std::cout << "Triggering null pointer exception in object member function" << std::endl;
				asutils::NativeCall(*pFunction, *pContext);
			}

			{
				CASOwningContext context(*pEngine);
				//Test the scheduler.
				pModule->GetScheduler().Think(10, *context.GetContext());
			}

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

			pEngine->ReturnContext(pContext);

			eventSystem.RemoveHandlersOfModule(*pModule->GetModule());

			//Remove the module.
			manager.GetModuleManager().RemoveModule( pModule );
		}
	}

	//Shut down the Angelscript engine, frees all resources.
	manager.Shutdown();

	spdlog::drop( as::log->name() );

	as::log.reset();

	//Wait for input.
	getchar();

	return 0;
}