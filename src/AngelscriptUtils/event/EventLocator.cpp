#include "AngelscriptUtils/event/EventLocator.h"
#include "AngelscriptUtils/event/EventSystem.h"

#include "AngelscriptUtils/utility/Objects.h"
#include "AngelscriptUtils/utility/TypeStringUtils.h"

namespace asutils
{
EventLocator::EventLocator(asITypeInfo* type, EventSystem* eventSystem)
	: m_Engine(type->GetEngine())
{
	if (!eventSystem)
	{
		auto context = asGetActiveContext();

		context->SetException("Event system handle cannot be null", false);
	}

	auto& eventType = *type->GetSubType();

	m_EventData = eventSystem->Lookup(eventType);

	if (!m_EventData)
	{
		auto context = asGetActiveContext();

		context->SetException((std::string{"The type \""} +
			FormatObjectTypeName(eventType.GetNamespace(), eventType.GetName()) + "\" is not an event type").c_str(), false);
	}
}

bool EventLocator::IsSubscribed(void* object, int typeId) const
{
	auto function = GetFunctionFromVariable(object, typeId);

	if (function)
	{
		return IsSubscribed(*function);
	}

	return false;
}

bool EventLocator::IsSubscribed(asIScriptFunction& function) const
{
	return m_EventData->IsSubscribed(function);
}

void EventLocator::Subscribe(void* object, int typeId)
{
	auto function = GetFunctionFromVariable(object, typeId);

	if (function)
	{
		m_EventData->Subscribe(*function);
	}
}

void EventLocator::Unsubscribe(void* object, int typeId)
{
	auto function = GetFunctionFromVariable(object, typeId);

	if (function)
	{
		m_EventData->Unsubscribe(*function);
	}
}

asIScriptFunction* EventLocator::GetFunctionFromVariable(void* object, int typeId) const
{
	auto function = TryGetFunctionFromVariableParameter(*m_Engine, object, typeId);

	if (!function)
	{
		auto context = asGetActiveContext();

		context->SetException("Event subscriber must be a function handle", false);
	}

	return function;
}
}
