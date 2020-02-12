#pragma once

#include "AngelscriptUtils/util/CASBaseClass.h"

namespace asutils
{
/**
*	@brief Base class for event arguments
*/
class EventArgs : public CASRefCountedBaseClass
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
}
