#include <algorithm>
#include <string>

#include <angelscript.h>

#include "AngelscriptUtils/event/Event.h"
#include "AngelscriptUtils/event/EventRegistry.h"

namespace asutils
{
Event::Event(const EventMetaData& metaData, const CASRefPtr<asIScriptContext>& context)
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
		return std::find_if(m_Functions.begin(), m_Functions.end(), [&](const auto& ptr)
			{
				return ptr->GetDelegateObject() == function.GetDelegateObject() &&
					ptr->GetDelegateFunction() == function.GetDelegateFunction();
			}) != m_Functions.end();
	}
	else
	{
		return std::find_if(m_Functions.begin(), m_Functions.end(), [&](const auto& ptr)
			{
				return ptr.Get() == &function;
			}) != m_Functions.end();
	}
}

void Event::Subscribe(asIScriptFunction& function)
{
	if (!ValidateFunctionFormat(function))
	{
		auto context = asGetActiveContext();

		auto name = GetMetaData().type.GetName();

		context->SetException((std::string{"Event handler must have the format void "} +name + "Callback(" + name + "@)").c_str(), false);
		return;
	}

	//TODO: verify that the function is compatible with this event
	if (!IsSubscribed(function))
	{
		m_Functions.emplace_back(&function);
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
		m_Functions.erase(std::find_if(m_Functions.begin(), m_Functions.end(), [&](const auto& ptr)
			{
				return ptr->GetDelegateObject() == function.GetDelegateObject() &&
					ptr->GetDelegateFunction() == function.GetDelegateFunction();
			}));
	}
	else
	{
		m_Functions.erase(std::find_if(m_Functions.begin(), m_Functions.end(), [&](const auto& ptr)
			{
				return ptr.Get() == &function;
			}));
	}
}

void Event::Dispatch(EventArgs& arguments)
{
	for (auto function : m_Functions)
	{
		//TODO: error handling
		m_Context->Prepare(function.Get());

		m_Context->SetArgObject(0, &arguments);

		m_Context->Execute();
	}

	m_Context->Unprepare();
}

void Event::RemoveFunctionsOfModule(asIScriptModule& module)
{
	m_Functions.erase(std::remove_if(m_Functions.begin(), m_Functions.end(), [&](const auto& function)
		{
			if (function->GetFuncType() == asFUNC_DELEGATE)
			{
				return function->GetDelegateFunction()->GetModule() == &module;
			}
			else
			{
				return function->GetModule() == &module;
			}
		}), m_Functions.end());
}
}
