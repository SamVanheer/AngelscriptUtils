#pragma once

#include "AngelscriptUtils/utility/BaseClasses.h"

namespace asutils
{
/**
*	@brief Base class for event arguments
*/
class EventArgs : public ReferenceCountedClass
{
protected:
	EventArgs()
	{
	}
public:
	virtual ~EventArgs()
	{
	}

	//Must be copy constructible
	EventArgs(const EventArgs&) = default;
	EventArgs& operator=(const EventArgs&) = delete;

	void Release() const
	{
		if (InternalRelease())
		{
			delete this;
		}
	}
};

/**
*	@brief Base class for events that can be preempted by handlers by setting the Handled property to true
*/
class PreemptableEventArgs : public EventArgs
{
public:

	bool IsHandled() const { return m_Handled; }

	void SetHandled(bool value)
	{
		m_Handled = value;
	}

private:
	bool m_Handled = false;
};
}
