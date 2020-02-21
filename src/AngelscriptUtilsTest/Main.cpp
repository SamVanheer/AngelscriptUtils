#include <iostream>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/daily_file_sink.h>

#include <angelscript.h>

#include "add_on/scriptany/scriptany.h"
#include "add_on/scriptarray/scriptarray.h"
#include "add_on/scriptbuilder/scriptbuilder.h"
#include "add_on/scriptdictionary/scriptdictionary.h"
#include "add_on/scriptstdstring/scriptstdstring.h"

#include "AngelscriptUtils/compilation/GlobalVariablesList.h"

#include "AngelscriptUtils/event/EventArgs.h"
#include "AngelscriptUtils/event/EventScriptAPI.h"
#include "AngelscriptUtils/event/EventSystem.h"

#include "AngelscriptUtils/execution/Metadata.h"
#include "AngelscriptUtils/execution/Execution.h"
#include "AngelscriptUtils/execution/Packing.h"

#include "AngelscriptUtils/ScriptAPI/Scheduler.h"
#include "AngelscriptUtils/ScriptAPI/Reflection/ReflectionScriptAPI.h"

#include "AngelscriptUtils/utility/MessageLogging.h"
#include "AngelscriptUtils/utility/Objects.h"
#include "AngelscriptUtils/utility/SmartPointers.h"
#include "AngelscriptUtils/utility/Variant.h"

#include "AngelscriptUtils/wrapper/WrappedScriptContext.h"

#include "CBaseEntity.h"
#include "CScriptBaseEntity.h"
#include "ASCBaseEntity.h"

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
	NONE = 0,

	/**
	*	Shared API.
	*/
	SHARED = 1 << 0,

	/**
	*	Map script specific.
	*/
	MAPSCRIPT_ONLY = 1 << 1,

	MAPSCRIPT = SHARED | MAPSCRIPT_ONLY,

	/**
	*	Plugin script specific.
	*/
	PLUGIN_ONLY = 1 << 2,

	PLUGIN = SHARED | PLUGIN_ONLY,

	/**
	*	All scripts.
	*/
	ALL = SHARED | MAPSCRIPT | PLUGIN
};
}

void Print(const std::string& szString)
{
	std::cout << szString;
}

void SuspendTest()
{
	auto context = asGetActiveContext();

	context->Suspend();
}

int NSTest()
{
	return 0;
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

DEFINE_OBJECT_TYPE_SIMPLE(MyEvent, MyEvent, asOBJ_REF)

enum class EnumType
{
	Value = 1
};

DEFINE_ENUM_TYPE_SIMPLE(EnumType, EnumType)

void RegisterMyEvent(asIScriptEngine& engine)
{
	const asutils::ObjectType<MyEvent> type;

	const auto className = type.GetName();

	asutils::RegisterEventClass<MyEvent>(engine);

	engine.RegisterObjectMethod(className, "bool get_ShouldHide() const property", asMETHOD(MyEvent, ShouldHide), asCALL_THISCALL);
	engine.RegisterObjectMethod(className, "void set_ShouldHide(bool value) property", asMETHOD(MyEvent, SetShouldHide), asCALL_THISCALL);
}

constexpr asPWORD MODULE_USER_DATA_ID = 10001;

struct ModuleUserData final
{
	ModuleUserData()
		: Scheduler(std::make_unique<asutils::Scheduler>())
	{
	}

	void PreDiscardCleanup()
	{
		Scheduler.reset();
	}

	std::unique_ptr<asutils::Scheduler> Scheduler;
};

void CleanupModuleUserData(asIScriptModule* module)
{
	auto data = reinterpret_cast<ModuleUserData*>(module->GetUserData(MODULE_USER_DATA_ID));

	delete data;
}

/**
*	@brief Builder for the test script
*/
class TestModuleBuilder
{
public:
	TestModuleBuilder(const std::string& szDecl)
		: m_szDecl(szDecl)
	{
	}

	bool AddScripts(CScriptBuilder& builder)
	{
		//By using a handle this can be changed, but since there are no other instances, it can only be made null.
		//TODO: figure out a better way.
		m_GlobalVariables.Add("ScriptScheduler@", "Scheduler",
			std::bind(&TestModuleBuilder::SetScheduler, this, std::placeholders::_1));

		const auto globalsSection = m_GlobalVariables.GetDeclarationsAsSection();

		auto result = builder.AddSectionFromMemory(
			"__Globals",
			globalsSection.c_str());

		if (result < 0)
			return false;

		if (!m_szDecl.empty() && builder.AddSectionFromMemory(
			"__CScriptBaseEntity",
			m_szDecl.c_str()) < 0)
			return false;

		return builder.AddSectionFromFile("resources/scripts/test.as") >= 0;
	}

	bool PostBuild(asIScriptModule& module)
	{
		auto userData = new ModuleUserData();
		module.SetUserData(userData, MODULE_USER_DATA_ID);

		return m_GlobalVariables.InitializeAll(module, userData);
	}

	asIScriptModule* Build(asIScriptEngine& engine, const char* name, const asDWORD accessMask)
	{
		CScriptBuilder builder;

		if (builder.StartNewModule(&engine, name) < 0)
		{
			return nullptr;
		}

		auto module = builder.GetModule();

		std::unique_ptr<asIScriptModule, decltype(&TestModuleBuilder::DiscardModule)> deleter(module, &TestModuleBuilder::DiscardModule);

		module->SetAccessMask(accessMask);

		if (!AddScripts(builder))
		{
			return nullptr;
		}

		const auto success = builder.BuildModule() >= 0;

		if (!PostBuild(*module))
		{
			return nullptr;
		}

		return deleter.release();
	}

private:
	static void DiscardModule(asIScriptModule* module)
	{
		module->Discard();
	}

	bool SetScheduler(const asutils::GlobalInitializerData& data)
	{
		auto userData = reinterpret_cast<ModuleUserData*>(data.UserData);

		return asutils::SetGlobalByName(data.Module, data.VariableName, userData->Scheduler.get());
	}

private:
	std::string m_szDecl;

	asutils::GlobalVariablesList m_GlobalVariables;
};

class TestInitializer
{
public:
	TestInitializer()
		: m_Engine(nullptr, &TestInitializer::ShutdownAndReleaseEngine)
	{
		m_Logger = CreateLogger();

		spdlog::register_logger(m_Logger);
	}

	~TestInitializer()
	{
		spdlog::drop(m_Logger->name());
	}

	asIScriptEngine& GetEngine()
	{
		return *m_Engine;
	}

	asutils::EventSystem& GetEventSystem()
	{
		return *m_EventSystem;
	}

	bool Initialize()
	{
		m_Engine.reset(asCreateScriptEngine(ANGELSCRIPT_VERSION));

		if (!m_Engine)
		{
			return false;
		}

		m_Engine->SetMessageCallback(asFUNCTION(&TestInitializer::TestMessageCallback), nullptr, asCALL_CDECL);

		m_Engine->SetContextCallbacks(&TestInitializer::CreateScriptContext, &TestInitializer::DestroyScriptContext, this);

		m_Engine->SetModuleUserDataCleanupCallback(&CleanupModuleUserData, MODULE_USER_DATA_ID);

		if (!RegisterAPI())
		{
			return false;
		}

		return true;
	}

private:
	bool RegisterAPI()
	{
		auto& engine = *m_Engine;

		RegisterStdString(&engine);
		RegisterScriptArray(&engine, true);
		RegisterScriptDictionary(&engine);
		RegisterScriptAny(&engine);
		asutils::RegisterSchedulerAPI(engine);
		asutils::RegisterReflectionAPI(engine);

		asutils::RegisterEventAPI(engine, true);

		RegisterMyEvent(engine);

		m_EventSystem = std::make_unique<asutils::EventSystem>(asutils::ReferencePointer<asIScriptContext>(engine.RequestContext(), true));

		engine.RegisterGlobalProperty("EventSystem g_EventSystem", m_EventSystem.get());

		engine.RegisterTypedef("size_t", "uint32");

		//Printing function.
		engine.RegisterGlobalFunction("void Print(const string& in szString)", asFUNCTION(Print), asCALL_CDECL);

		engine.RegisterGlobalFunction("void SuspendTest()", asFUNCTION(SuspendTest), asCALL_CDECL);

		engine.SetDefaultNamespace("NS");

		engine.RegisterGlobalFunction(
			"int NSTest()",
			asFUNCTION(NSTest),
			asCALL_CDECL);

		engine.SetDefaultNamespace("");

		//Register the interface that all custom entities use. Allows you to take them as handles to functions.
		engine.RegisterInterface("IScriptEntity");

		//Register the entity class.
		RegisterScriptCBaseEntity(engine);

		//Register the entity base class. Used to call base class implementations.
		RegisterScriptBaseEntity(engine);

		return true;
	}

	static std::shared_ptr<spdlog::logger> CreateLogger()
	{
		auto console = std::make_shared<spdlog::sinks::stdout_sink_mt>();
		auto file = std::make_shared<spdlog::sinks::daily_file_sink_mt>("logs/L.log", 0, 0);

		return std::make_shared<spdlog::logger>("ASUtils", spdlog::sinks_init_list{console, file});
	}

	static void ShutdownAndReleaseEngine(asIScriptEngine* engine)
	{
		engine->ShutDownAndRelease();
	}

	static void TestMessageCallback(const asSMessageInfo* message, void*)
	{
		asutils::FormatEngineMessage(*message, std::cout);
	}

	static asIScriptContext* CreateScriptContext(asIScriptEngine* engine, void* initializer)
	{
		auto context = engine->CreateContext();

		//TODO: add test to see if suspending will log an error.
		auto wrapper = new asutils::LoggingScriptContext(*context, reinterpret_cast<TestInitializer*>(initializer)->m_Logger, true);

		context->Release();

		return wrapper;
	}

	static void DestroyScriptContext(asIScriptEngine*, asIScriptContext* context, void*)
	{
		//Releases both the wrapper and context at once if this is a wrapped context
		if (context)
		{
			context->Release();
		}
	}

private:
	std::shared_ptr<spdlog::logger> m_Logger;

	std::unique_ptr<asIScriptEngine, decltype(&TestInitializer::ShutdownAndReleaseEngine)> m_Engine;

	std::unique_ptr<asutils::EventSystem> m_EventSystem;

private:
	TestInitializer(const TestInitializer&) = delete;
	TestInitializer& operator=(const TestInitializer&) = delete;
};

int main(int, char* [])
{
	std::cout << "Hello World!" << std::endl;

	if (TestInitializer initializer; initializer.Initialize())
	{
		auto& engine = initializer.GetEngine();

		asutils::Variant variant(10);

		variant.Reset(10);

		initializer.GetEventSystem().Register<MyEvent>(*engine.GetTypeInfoByName("MyEvent"));

		//Create the declaration used for script entity base classes.
		/*
		const auto szDecl = as::CreateExtendBaseclassDeclaration( "CScriptBaseEntity", "IScriptEntity", "CBaseEntity", "BaseEntity" );

		std::cout << szDecl << std::endl;
		*/
		const std::string szDecl{};

		//Make a map script.
		TestModuleBuilder builder(szDecl);

		auto mapModule = builder.Build(engine, "MapScript", ModuleAccessMask::MAPSCRIPT);

		if (mapModule)
		{
			auto& module = *mapModule;

			auto userData = reinterpret_cast<ModuleUserData*>(module.GetUserData(MODULE_USER_DATA_ID));

			auto context = engine.RequestContext();

			auto& eventSystem = initializer.GetEventSystem();

			if (auto function = module.GetFunctionByName("TemplatedCallTest"))
			{
				auto parameters = asutils::CreateNativeParameterList(10, EnumType::Value, new MyEvent(), std::string{"Packed parameters"});

				asutils::PackedCall(*function, *context, parameters);

				int output = 10;
				auto event = new MyEvent();
				event->AddRef();
				asutils::NativeCall(*function, *context, output, EnumType::Value, event, std::string{"Test string"});

				std::cout << "C++ value of foo is " << output << std::endl;
			}

			//Call the main function.
			if (auto function = module.GetFunctionByName("main"))
			{
				std::string szString = "Hello World!\n";

				//Note: main takes a const string& in. We can pass by pointer or by reference, either will work the same way
				//Constructing the string in the parameter list itself also works
				asutils::NativeCall(*function, *context, &szString);

				MyEvent event;

				eventSystem.GetEvent<MyEvent>().Dispatch(event);

				std::cout << "Should hide: " << event.ShouldHide() << std::endl;
			}

			//Test the object pointer.
			if (auto function = module.GetFunctionByName("GetLifetime"))
			{
				{
					asutils::FunctionExecutor executor(*context);

					auto functionCaller = executor.Global(*function);

					if (functionCaller.NativeCall())
					{
						asutils::ObjectPointer ptr;

						auto result = functionCaller.GetObjectReturnValue();

						if (result)
						{
							const int iTypeId = function->GetReturnTypeId();

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
			if (auto function = module.GetFunctionByName("NoArgs"))
			{
				{
					//Test the smart pointer.
					asutils::ReferencePointer<asIScriptFunction> func;

					asutils::ReferencePointer<asIScriptFunction> func2(function);

					func = func2;

					func = std::move(func2);

					asutils::ReferencePointer<asIScriptFunction> func3(func);

					asutils::ReferencePointer<asIScriptFunction> func4(std::move(func));

					func.Reset(function);

					auto ptr = func.Get();

					std::cout << "Smart pointer points to: 0x" << std::hex << reinterpret_cast<intptr_t>(ptr) << std::dec << std::endl;
				}

				//Regular variadic templates
				asutils::NativeCall(*function, *context);

				//Argument list
				auto parameterList = asutils::CreateNativeParameterList();
				asutils::PackedCall(*function, *context, parameterList);

				//Wrapper version
				{
					asutils::FunctionExecutor executor{*context};

					executor.Global(*function).NativeCall();
				}
			}

			//Call a function that triggers a null pointer exception.
			if (auto function = module.GetFunctionByName("DoNullPointerException"))
			{
				std::cout << "Triggering null pointer exception" << std::endl;
				asutils::NativeCall(*function, *context);
			}

			if (auto function = module.GetFunctionByName("DoNullPointerException2"))
			{
				std::cout << "Triggering null pointer exception in object member function" << std::endl;
				asutils::NativeCall(*function, *context);
			}

			if (auto function = module.GetFunctionByName("TestSuspendCall"))
			{
				std::cout << "Calling TestSuspendCall" << std::endl;

				asutils::FunctionExecutor executor{*context};

				executor.Global(*function).NativeCall();

				const auto resumeResult = executor.GetContext().Execute();

				std::cout << "Resumed execution: " << ((resumeResult >= 0) ? "success" : "failure") << std::endl;
			}

			{
				//Test the scheduler.
				userData->Scheduler->Think(10, *context);
			}

			//Get the parameter types. Angelscript's type info support isn't complete yet, so not all types have an asITypeInfo instance yet.
			
			/*
			if(auto function2 = module.GetFunctionByName("Function"))
			{
				for(asUINT index = 0; index < function2->GetParamCount(); ++index)
				{
					int typeId;
					const char* name;
					function2->GetParam(index, &typeId, nullptr, &name);

					std::cout << "Parameter " << index << ": " << name << std::endl;

					if(auto type = engine.GetTypeInfoById(typeId))
					{
						std::cout << type->GetNamespace() << "::" << type->GetName() << std::endl;

						asDWORD flags = type->GetFlags();

						if(flags & asOBJ_VALUE)
							std::cout << "Value" << std::endl;

						if(flags & asOBJ_REF)
							std::cout << "Ref" << std::endl;

						if(flags & asOBJ_ENUM)
							std::cout << "Enum" << std::endl;

						if(flags & asOBJ_FUNCDEF)
							std::cout << "Funcdef" << std::endl;

						if(flags & asOBJ_POD)
							std::cout << "POD" << std::endl;

						if(flags & asOBJ_TYPEDEF)
							std::cout << "Typedef" << std::endl;
					}
					else //Only primitive types don't have type info right now.
						std::cout << "No type info" << std::endl;
				}
			}
			*/

			/*
			//Try to create a C++ class that is extended in a script.
			bool createdExtend = false;

			//This data will need to be stored somewhere, bound to the baseclass.
			if(auto entity = as::CreateExtensionClassInstance<CScriptBaseEntity>(engine, module, "CEntity", "CBaseEntity", "BaseEntity"))
			{
				createdExtend = true;

				CBaseEntity* baseEntity = entity;

				baseEntity->Spawn();

				int result = baseEntity->ScheduleOfType( "foo" );

				//Silence compiler warnings (unused var).
				result = result;

				delete entity;
			}

			std::cout << "Created extend class: " << (createdExtend ? "yes" : "no") << std::endl;
			*/

			engine.ReturnContext(context);

			eventSystem.RemoveHandlersOfModule(module);

			//Remove the module.
			//The scheduler has to be reset before discarding since the engine checks for references before running the cleanup callback
			userData->PreDiscardCleanup();
			mapModule->Discard();
		}

		//Initializer goes out of scope here and frees all resources automatically
	}

	//Wait for input.
	getchar();

	return 0;
}