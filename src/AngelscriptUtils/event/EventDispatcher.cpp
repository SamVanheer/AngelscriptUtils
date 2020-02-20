#include "AngelscriptUtils/event/EventDispatcher.h"
#include "AngelscriptUtils/event/EventSystem.h"

namespace asutils
{
void BaseEventDispatcher::Dispatch(EventArgs& arguments)
{
	for (asUINT index = 0; index < m_EventHandlers.GetCount(); ++index)
	{
		auto result = m_Context->Prepare(m_EventHandlers.GetFunction(index));

		if (result >= 0)
		{
			result = m_Context->SetArgObject(0, &arguments);
		}

		if (result >= 0)
		{
			result = m_Context->Execute();
		}

		if (result < 0)
		{
			//TODO: log that we're stopping
			break;
		}
	}

	m_Context->Unprepare();
}

void BaseEventDispatcher::Dispatch(PreemptableEventArgs& arguments)
{
	for (asUINT index = 0; index < m_EventHandlers.GetCount(); ++index)
	{
		auto result = m_Context->Prepare(m_EventHandlers.GetFunction(index));

		if (result >= 0)
		{
			result = m_Context->SetArgObject(0, &arguments);
		}

		if (result >= 0)
		{
			result = m_Context->Execute();
		}

		if (result < 0)
		{
			//TODO: log that we're stopping
			break;
		}

		if (arguments.IsHandled())
		{
			break;
		}
	}

	m_Context->Unprepare();
}
}
