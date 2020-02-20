#include "AngelscriptUtils/event/EventLocator.h"
#include "AngelscriptUtils/event/EventScriptAPI.h"
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
	auto& eventHandlers = m_EventData->EventHandlers;

	if (function.GetFuncType() == asFUNC_DELEGATE)
	{
		//Test for the underlying method instead
		return std::find_if(eventHandlers.begin(), eventHandlers.end(), [&](const auto& ptr)
			{
				return ptr->GetDelegateObject() == function.GetDelegateObject() &&
					ptr->GetDelegateFunction() == function.GetDelegateFunction();
			}) != eventHandlers.end();
	}
	else
	{
		return std::find_if(eventHandlers.begin(), eventHandlers.end(), [&](const auto& ptr)
			{
				return ptr.Get() == &function;
			}) != eventHandlers.end();
	}
}

void EventLocator::Subscribe(void* object, int typeId)
{
	auto function = GetFunctionFromVariable(object, typeId);

	if (function)
	{
		if (!ValidateFunctionFormat(*function))
		{
			auto context = asGetActiveContext();

			context->SetException(("Event handler must have the format " +
				FormatEventHandlerFuncdef(m_EventData->EventType->GetName())).c_str(), false);
			return;
		}

		if (!IsSubscribed(*function))
		{
			m_EventData->EventHandlers.emplace_back(function);
		}
	}
}

void EventLocator::Unsubscribe(void* object, int typeId)
{
	auto function = GetFunctionFromVariable(object, typeId);

	if (function)
	{
		auto& eventHandlers = m_EventData->EventHandlers;

		if (function->GetFuncType() == asFUNC_DELEGATE)
		{
			//Test for the underlying method instead
			eventHandlers.erase(std::find_if(eventHandlers.begin(), eventHandlers.end(), [&](const auto& ptr)
				{
					return ptr->GetDelegateObject() == function->GetDelegateObject() &&
						ptr->GetDelegateFunction() == function->GetDelegateFunction();
				}));
		}
		else
		{
			eventHandlers.erase(std::find_if(eventHandlers.begin(), eventHandlers.end(), [&](const auto& ptr)
				{
					return ptr.Get() == function;
				}));
		}
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

bool EventLocator::ValidateFunctionFormat(asIScriptFunction& function) const
{
	if (function.GetReturnTypeId() != asTYPEID_VOID)
	{
		return false;
	}

	if (function.GetParamCount() != 1)
	{
		return false;
	}

	int typeId;

	function.GetParam(0, &typeId);

	if (!(typeId & asTYPEID_OBJHANDLE))
	{
		return false;
	}

	typeId &= ~asTYPEID_OBJHANDLE;

	if (m_EventData->EventType->GetTypeId() != typeId)
	{
		return false;
	}

	return true;
}
}
