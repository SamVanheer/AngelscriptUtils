#pragma once

#include <angelscript.h>

#include "AngelscriptUtils/event/EventArgs.h"

#include "AngelscriptUtils/utility/SmartPointers.h"

namespace asutils
{
class IEventHandlers;

class BaseEventDispatcher
{
protected:
	BaseEventDispatcher(IEventHandlers& eventHandlers, ReferencePointer<asIScriptContext>&& context)
		: m_EventHandlers(eventHandlers)
		, m_Context(std::move(context))
	{
	}

	/**
	*	@brief Dispatches the given arguments object to all listeners
	*/
	void Dispatch(EventArgs& arguments);

	/**
	*	@brief Dispatches the given arguments object to all listeners
	*/
	void Dispatch(PreemptableEventArgs& arguments);

public:
	BaseEventDispatcher(const BaseEventDispatcher&) = delete;
	BaseEventDispatcher& operator=(const BaseEventDispatcher&) = delete;

	BaseEventDispatcher(BaseEventDispatcher&&) = default;

private:
	IEventHandlers& m_EventHandlers;
	const ReferencePointer<asIScriptContext> m_Context;
};

template<typename T>
class EventDispatcher final : public BaseEventDispatcher
{
public:
	EventDispatcher(IEventHandlers& eventHandlers, ReferencePointer<asIScriptContext>&& context)
		: BaseEventDispatcher(eventHandlers, std::move(context))
	{
	}

	EventDispatcher(EventDispatcher&&) = default;

	/**
	*	@brief Dispatches an event of type T with arguments args
	*	@tparam T Type of the event to dispatch. Must be a type that inherits from EventArgs
	*/
	void Dispatch(const T& args)
	{
		//Create a dynamically allocated instance so scripts won't retain a reference to the stack
		ReferencePointer<T> object{new T(args)};

		BaseEventDispatcher::Dispatch(*object);
	}

	/**
	*	@brief Dispatches an event of type T with arguments args
	*	This overload allows you to pass an event object directly and get any changes back in the original object
	*	Take care to dynamically allocate the object to avoid allowing scripts to retain references to it
	*	@tparam T Type of the event to dispatch. Must be a type that inherits from EventArgs
	*/
	void Dispatch(T& args)
	{
		BaseEventDispatcher::Dispatch(args);
	}
};
}
