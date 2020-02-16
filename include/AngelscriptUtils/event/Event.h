#pragma once

#include <vector>

#include "AngelscriptUtils/util/CASBaseClass.h"
#include "AngelscriptUtils/utility/SmartPointers.h"

#include "AngelscriptUtils/event/EventArgs.h"

class asIScriptContext;
class asIScriptFunction;

namespace asutils
{
struct EventMetaData;

/**
*	@brief Represents an event, stores handlers, handles dispatching
*/
class Event : public CASRefCountedBaseClass
{
protected:
	Event(const EventMetaData& metaData, const ReferencePointer<asIScriptContext>& context);

public:
	virtual ~Event();

	Event(const Event&) = delete;
	Event& operator=(const Event&) = delete;

	const EventMetaData& GetMetaData() const { return m_MetaData; }

	bool IsSubscribed(asIScriptFunction& function) const;

	void Subscribe(asIScriptFunction& function);

	void Unsubscribe(asIScriptFunction& function);

	void Release() const
	{
		if (InternalRelease())
		{
			delete this;
		}
	}

	/**
	*	@brief Removes all event handlers defined in the given module from this event
	*/
	void RemoveHandlersOfModule(asIScriptModule& module);

	void RemoveAllHandlers();

protected:
	/**
	*	@brief Dispatches the given arguments object to all listeners
	*/
	void Dispatch(EventArgs& arguments);

	/**
	*	@brief Dispatches the given arguments object to all listeners
	*/
	void Dispatch(PreemptableEventArgs& arguments);

private:
	bool ValidateFunctionFormat(asIScriptFunction& function) const;

private:
	const EventMetaData& m_MetaData;

	//TODO: could store a reference to the event system and allow the context to be swapped out there
	ReferencePointer<asIScriptContext> m_Context;

	std::vector<ReferencePointer<asIScriptFunction>> m_EventHandlers;
};

/**
*	@brief Type-safe event for C++ event dispatching
*/
template<typename T>
class TypedEvent : public Event
{
public:
	TypedEvent(const EventMetaData& metaData, const ReferencePointer<asIScriptContext>& context)
		: Event(metaData, context)
	{
	}

	/**
	*	@brief Dispatches an event of type T with arguments args
	*	@tparam T Type of the event to dispatch. Must be a type that inherits from EventArgs
	*/
	void Dispatch(const T& args)
	{
		//Create a dynamically allocated instance so scripts won't retain a reference to the stack
		ReferencePointer<T> object{new T(args)};

		Event::Dispatch(*object);
	}

	/**
	*	@brief Dispatches an event of type T with arguments args
	*	This overload allows you to pass an event object directly and get any changes back in the original object
	*	Take care to dynamically allocate the object to avoid allowing scripts to retain references to it
	*	@tparam T Type of the event to dispatch. Must be a type that inherits from EventArgs
	*/
	void Dispatch(T& args)
	{
		Event::Dispatch(args);
	}
};
}
