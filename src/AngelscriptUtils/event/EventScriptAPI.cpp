#include <new>
#include <string>

#include <angelscript.h>

#include "AngelscriptUtils/event/EventArgs.h"
#include "AngelscriptUtils/event/EventLocator.h"
#include "AngelscriptUtils/event/EventSystem.h"

namespace asutils
{
void RegisterEventSystem(asIScriptEngine& engine)
{
	const auto className = "EventSystem";

	engine.RegisterObjectType(className, sizeof(EventSystem), asOBJ_REF | asOBJ_NOCOUNT);
}

namespace
{
void ConstructEventLocator(void* memory, asITypeInfo* type, EventSystem* eventSystem)
{
	new(memory) EventLocator(type, eventSystem);
}

void DestructEventLocator(EventLocator* memory)
{
	memory->~EventLocator();
}

void RegisterEventLocator(asIScriptEngine& engine, const bool provideEventSystemInstance)
{
	engine.RegisterObjectType("Event<class TEvent>", sizeof(EventLocator), asOBJ_VALUE | asOBJ_TEMPLATE | asGetTypeTraits<EventLocator>());

	const auto className = "Event<TEvent>";

	if (provideEventSystemInstance)
	{
		engine.RegisterObjectBehaviour(className, asBEHAVE_CONSTRUCT, "void Event(int& in, EventSystem@ eventSystem)",
			asFUNCTIONPR(ConstructEventLocator, (void*, asITypeInfo*, EventSystem*), void), asCALL_CDECL_OBJFIRST);
	}

	engine.RegisterObjectBehaviour(className, asBEHAVE_DESTRUCT, "void Event()",
		asFUNCTION(DestructEventLocator), asCALL_CDECL_OBJFIRST);

	engine.RegisterObjectMethod(className, "void Subscribe(?& in)", asMETHOD(EventLocator, Subscribe), asCALL_THISCALL);
	engine.RegisterObjectMethod(className, "void Unsubscribe(?& in)", asMETHOD(EventLocator, Unsubscribe), asCALL_THISCALL);
}
}

void RegisterEventAPI(asIScriptEngine& engine, const bool provideEventSystemInstance)
{
	if (provideEventSystemInstance)
	{
		RegisterEventSystem(engine);
	}

	RegisterEventLocator(engine, provideEventSystemInstance);
}

std::string FormatEventHandlerFuncdef(const char* className)
{
	return std::string{"void "} + className + "Handler(" + className + "@ args)";
}
}