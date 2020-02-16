#include <algorithm>
#include <string>

#include <angelscript.h>

#include "AngelscriptUtils/event/Event.h"
#include "AngelscriptUtils/event/EventRegistry.h"
#include "AngelscriptUtils/event/EventScriptAPI.h"

namespace asutils
{
Event::Event(const EventMetaData& metaData, const ReferencePointer<asIScriptContext>& context)
	: m_MetaData(metaData)
	, m_Context(context)
{
}

Event::~Event() = default;

bool Event::IsSubscribed(asIScriptFunction& function) const
{
	if (function.GetFuncType() == asFUNC_DELEGATE)
	{
		//Test for the underlying method instead
		return std::find_if(m_EventHandlers.begin(), m_EventHandlers.end(), [&](const auto& ptr)
			{
				return ptr->GetDelegateObject() == function.GetDelegateObject() &&
					ptr->GetDelegateFunction() == function.GetDelegateFunction();
			}) != m_EventHandlers.end();
	}
	else
	{
		return std::find_if(m_EventHandlers.begin(), m_EventHandlers.end(), [&](const auto& ptr)
			{
				return ptr.Get() == &function;
			}) != m_EventHandlers.end();
	}
}

void Event::Subscribe(asIScriptFunction& function)
{
	if (!ValidateFunctionFormat(function))
	{
		auto context = asGetActiveContext();

		context->SetException(("Event handler must have the format " + FormatEventHandlerFuncdef(GetMetaData().type.GetName())).c_str(), false);
		return;
	}

	//TODO: verify that the function is compatible with this event
	if (!IsSubscribed(function))
	{
		m_EventHandlers.emplace_back(&function);
	}
}

bool Event::ValidateFunctionFormat(asIScriptFunction& function) const
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

	if (GetMetaData().type.GetTypeId() != typeId)
	{
		return false;
	}

	return true;
}

void Event::Unsubscribe(asIScriptFunction& function)
{
	if (function.GetFuncType() == asFUNC_DELEGATE)
	{
		//Test for the underlying method instead
		m_EventHandlers.erase(std::find_if(m_EventHandlers.begin(), m_EventHandlers.end(), [&](const auto& ptr)
			{
				return ptr->GetDelegateObject() == function.GetDelegateObject() &&
					ptr->GetDelegateFunction() == function.GetDelegateFunction();
			}));
	}
	else
	{
		m_EventHandlers.erase(std::find_if(m_EventHandlers.begin(), m_EventHandlers.end(), [&](const auto& ptr)
			{
				return ptr.Get() == &function;
			}));
	}
}

void Event::Dispatch(EventArgs& arguments)
{
	for (auto function : m_EventHandlers)
	{
		//TODO: error handling
		m_Context->Prepare(function.Get());

		m_Context->SetArgObject(0, &arguments);

		m_Context->Execute();
	}

	m_Context->Unprepare();
}

void Event::Dispatch(PreemptableEventArgs& arguments)
{
	for (auto function : m_EventHandlers)
	{
		//TODO: error handling
		m_Context->Prepare(function.Get());

		m_Context->SetArgObject(0, &arguments);

		m_Context->Execute();

		if (arguments.IsHandled())
		{
			break;
		}
	}

	m_Context->Unprepare();
}

void Event::RemoveHandlersOfModule(asIScriptModule& module)
{
	m_EventHandlers.erase(std::remove_if(m_EventHandlers.begin(), m_EventHandlers.end(), [&](const auto& function)
		{
			if (function->GetFuncType() == asFUNC_DELEGATE)
			{
				return function->GetDelegateFunction()->GetModule() == &module;
			}
			else
			{
				return function->GetModule() == &module;
			}
		}), m_EventHandlers.end());
}

void Event::RemoveAllHandlers()
{
	m_EventHandlers.clear();
}
}
