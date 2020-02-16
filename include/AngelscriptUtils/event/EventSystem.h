#pragma once

#include <functional>
#include <memory>
#include <stdexcept>
#include <typeinfo>
#include <unordered_map>

#include "AngelscriptUtils/std_make_unique.h"
#include "AngelscriptUtils/event/Event.h"
#include "AngelscriptUtils/event/EventRegistry.h"
#include "AngelscriptUtils/utility/SmartPointers.h"

namespace asutils
{
/**
*	@brief Stores events and handles event dispatch
*/
class EventSystem final
{
public:
	EventSystem(EventRegistry& registry, const ReferencePointer<asIScriptContext>& context);
	~EventSystem();

	EventSystem(const EventSystem&) = delete;
	EventSystem& operator=(const EventSystem&) = delete;

	/**
	*	@brief Gets an event by C++ type
	*/
	template<typename T, std::enable_if_t<std::is_base_of<EventArgs, T>::value, int> = 0>
	TypedEvent<T>& GetEvent()
	{
		auto metaData = m_Registry.Lookup<T>();

		if (metaData == nullptr)
		{
			throw std::invalid_argument("The type T is an unregistered event type");
		}

		return static_cast<TypedEvent<T>&>(InternalGetEvent(metaData));
	}

	/**
	*	@brief Gets an event by script type
	*	Should not be used directly
	*/
	bool TryGetEvent(const asITypeInfo& type, Event*& event)
	{
		auto metaData = m_Registry.Lookup(type);

		if (metaData == nullptr)
		{
			event = nullptr;
			return false;
		}

		event = &InternalGetEvent(metaData);

		return true;
	}

	/**
	*	@brief Removes all event handlers defined in the given module from all events
	*/
	void RemoveHandlersOfModule(asIScriptModule& module);

	void RemoveAllHandlers();

private:
	Event& InternalGetEvent(EventMetaData* metaData);

private:
	EventRegistry& m_Registry;

	ReferencePointer<asIScriptContext> m_Context;

	std::unordered_map<EventMetaData*, ReferencePointer<Event>> m_Events;
};
}