#include <new>
#include <string>

#include <angelscript.h>

#include "AngelscriptUtils/event/Event.h"
#include "AngelscriptUtils/event/EventArgs.h"
#include "AngelscriptUtils/event/EventSystem.h"

#include "AngelscriptUtils/utility/Objects.h"

namespace asutils
{
/**
*	@brief Angelscript template class used to locate events so scripts can subscribe and unsubscribe
*/
class EventLocator final
{
public:
	EventLocator(asITypeInfo* type, EventSystem* eventSystem)
		: m_Engine(*type->GetEngine())
	{
		auto& eventType = *type->GetSubType();

		if (!eventSystem->TryGetEvent(eventType, m_Event))
		{
			auto context = asGetActiveContext();

			context->SetException((std::string{"The type \""} + eventType.GetName() + "\" is not an event type").c_str(), false);
		}

		m_Engine.AddRef();
	}

	~EventLocator()
	{
		m_Engine.Release();
	}

	bool IsSubscribed(void* object, int typeId) const
	{
		auto function = GetFunctionFromVariable(object, typeId);

		if (function)
		{
			return m_Event->IsSubscribed(*function);
		}

		return false;
	}

	void Subscribe(void* object, int typeId)
	{
		auto function = GetFunctionFromVariable(object, typeId);

		if (function)
		{
			m_Event->Subscribe(*function);
		}
	}

	void Unsubscribe(void* object, int typeId)
	{
		auto function = GetFunctionFromVariable(object, typeId);

		if (function)
		{
			m_Event->Unsubscribe(*function);
		}
	}

private:
	asIScriptFunction* GetFunctionFromVariable(void* object, int typeId) const
	{
		auto function = InternalGetFunctionFromVariable(object, typeId);

		if (!function)
		{
			auto context = asGetActiveContext();

			context->SetException("Event subscriber must be a function handle", false);
		}

		return function;
	}

	asIScriptFunction* InternalGetFunctionFromVariable(void* object, int typeId) const
	{
		return TryGetFunctionFromVariableParameter(m_Engine, object, typeId);
	}

private:
	Event* m_Event;
	asIScriptEngine& m_Engine;
};

namespace
{
void RegisterEventSystem(asIScriptEngine& engine)
{
	const auto className = "EventSystem";

	engine.RegisterObjectType(className, sizeof(EventSystem), asOBJ_REF | asOBJ_NOCOUNT);
}

void ConstructEventLocator(void* memory, asITypeInfo* type, EventSystem* eventSystem)
{
	new(memory) EventLocator(type, eventSystem);
}

void DestructEventLocator(EventLocator* memory)
{
	memory->~EventLocator();
}

void RegisterEventLocator(asIScriptEngine& engine)
{
	engine.RegisterObjectType("Event<class TEvent>", sizeof(EventLocator), asOBJ_VALUE | asOBJ_TEMPLATE | asGetTypeTraits<EventLocator>());

	const auto className = "Event<TEvent>";

	engine.RegisterObjectBehaviour(className, asBEHAVE_CONSTRUCT, "void Event(int& in, EventSystem@ eventSystem)",
		asFUNCTIONPR(ConstructEventLocator, (void*, asITypeInfo*, EventSystem*), void), asCALL_CDECL_OBJFIRST);

	engine.RegisterObjectBehaviour(className, asBEHAVE_DESTRUCT, "void Event()",
		asFUNCTION(DestructEventLocator), asCALL_CDECL_OBJFIRST);

	engine.RegisterObjectMethod(className, "void Subscribe(?& in)", asMETHOD(EventLocator, Subscribe), asCALL_THISCALL);
	engine.RegisterObjectMethod(className, "void Unsubscribe(?& in)", asMETHOD(EventLocator, Unsubscribe), asCALL_THISCALL);
}
}

void RegisterEventAPI(asIScriptEngine& engine)
{
	RegisterEventSystem(engine);

	RegisterEventLocator(engine);
}

std::string FormatEventHandlerFuncdef(const char* className)
{
	return std::string{"void "} + className + "Handler(" + className + "@ args)";
}
}