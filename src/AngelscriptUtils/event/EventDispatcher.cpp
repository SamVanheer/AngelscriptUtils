#include <type_traits>

#include "AngelscriptUtils/event/EventDispatcher.h"
#include "AngelscriptUtils/event/EventSystem.h"

namespace asutils
{
template<typename T, std::enable_if_t<std::is_base_of_v<PreemptableEventArgs, T>, int> = 0>
bool IsEventHandled(const T& arguments)
{
	return arguments.IsHandled();
}

template<typename T, std::enable_if_t<!std::is_base_of_v<PreemptableEventArgs, T>, int> = 0>
bool IsEventHandled(const T&)
{
	return false;
}

template<typename T>
void DispatchEvent(IEventHandlers& eventHandlers, asIScriptContext& context, T& arguments)
{
	for (asUINT index = 0; index < eventHandlers.GetCount(); ++index)
	{
		auto result = context.Prepare(eventHandlers.GetFunction(index));

		if (result >= 0)
		{
			result = context.SetArgObject(0, &arguments);
		}

		if (result >= 0)
		{
			result = context.Execute();
		}

		if (result < 0)
		{
			context.GetEngine()->WriteMessage("asutils::DispatchEvent", 0, 0, asMSGTYPE_ERROR,
				"Stopping event dispatch due to context errors");
			break;
		}

		if (IsEventHandled(arguments))
		{
			break;
		}
	}

	context.Unprepare();
}

void BaseEventDispatcher::Dispatch(EventArgs& arguments)
{
	DispatchEvent(m_EventHandlers, *m_Context, arguments);
}

void BaseEventDispatcher::Dispatch(PreemptableEventArgs& arguments)
{
	DispatchEvent(m_EventHandlers, *m_Context, arguments);
}
}
