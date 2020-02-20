#include "AngelscriptUtils/event/EventScriptAPI.h"
#include "AngelscriptUtils/event/EventSystem.h"

namespace asutils
{
asUINT EventMetaData::GetCount() const
{
	return GetImmutableList().size();
}

asIScriptFunction* EventMetaData::GetFunction(asUINT index) const
{
	const auto& handlers = GetImmutableList();

	if (index < handlers.size())
	{
		return handlers[index].Get();
	}

	return nullptr;
}

void EventMetaData::SetExecuting(bool state)
{
	if (Executing != state)
	{
		Executing = state;

		if (!Executing)
		{
			ExecutingCopy.clear();
			ModifiedDuringExecution = false;
		}
	}
}

bool IsEventHandlerInList(asIScriptFunction& function, const std::vector<ReferencePointer<asIScriptFunction>>& list)
{
	if (function.GetFuncType() == asFUNC_DELEGATE)
	{
		//Test for the underlying method instead
		return std::find_if(list.begin(), list.end(), [&](const auto& ptr)
			{
				return ptr->GetDelegateObject() == function.GetDelegateObject() &&
					ptr->GetDelegateFunction() == function.GetDelegateFunction();
			}) != list.end();
	}
	else
	{
		return std::find_if(list.begin(), list.end(), [&](const auto& ptr)
			{
				return ptr.Get() == &function;
			}) != list.end();
	}
}

bool EventMetaData::IsSubscribed(asIScriptFunction& function) const
{
	return IsEventHandlerInList(function, EventHandlers);
}

void EventMetaData::Subscribe(asIScriptFunction& function)
{
	if (!ValidateFunctionFormat(function))
	{
		auto context = asGetActiveContext();

		context->SetException(("Event handler must have the format " +
			FormatEventHandlerFuncdef(EventType->GetName())).c_str(), false);
		return;
	}

	if (!IsSubscribed(function))
	{
		MakeCopyIfNeeded();

		EventHandlers.emplace_back(&function);
	}
}

void EventMetaData::Unsubscribe(asIScriptFunction& function)
{
	MakeCopyIfNeeded();

	if (function.GetFuncType() == asFUNC_DELEGATE)
	{
		//Test for the underlying method instead
		EventHandlers.erase(std::find_if(EventHandlers.begin(), EventHandlers.end(), [&](const auto& ptr)
			{
				return ptr->GetDelegateObject() == function.GetDelegateObject() &&
					ptr->GetDelegateFunction() == function.GetDelegateFunction();
			}));
	}
	else
	{
		EventHandlers.erase(std::find_if(EventHandlers.begin(), EventHandlers.end(), [&](const auto& ptr)
			{
				return ptr.Get() == &function;
			}));
	}
}

void EventMetaData::RemoveHandlersOfModule(asIScriptModule& module)
{
	EventHandlers.erase(std::remove_if(EventHandlers.begin(), EventHandlers.end(), [&](const auto& function)
		{
			if (function->GetFuncType() == asFUNC_DELEGATE)
			{
				return function->GetDelegateFunction()->GetModule() == &module;
			}
			else
			{
				return function->GetModule() == &module;
			}
		}), EventHandlers.end());
}

bool EventMetaData::ValidateFunctionFormat(asIScriptFunction& function) const
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

	if (EventType->GetTypeId() != typeId)
	{
		return false;
	}

	return true;
}
}
