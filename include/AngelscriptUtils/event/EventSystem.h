#pragma once

#include <functional>
#include <memory>
#include <stdexcept>
#include <typeinfo>
#include <unordered_map>

#include "AngelscriptUtils/std_make_unique.h"
#include "AngelscriptUtils/event/Event.h"
#include "AngelscriptUtils/event/EventRegistry.h"
#include "AngelscriptUtils/util/CASRefPtr.h"

namespace asutils
{
/**
*	@brief Stores events and handles event dispatch
*/
class EventSystem final
{
public:
	EventSystem(EventRegistry& registry, const CASRefPtr<asIScriptContext>& context);
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

		return static_cast<TypedEvent<T>&>(InternalGetEvent(metaData));
	}

	/**
	*	@brief Gets an event by script type
	*	Should not be used directly
	*/
	Event& GetEvent(const asITypeInfo& type)
	{
		auto metaData = m_Registry.Lookup(type);

		return InternalGetEvent(metaData);
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

	CASRefPtr<asIScriptContext> m_Context;

	std::unordered_map<EventMetaData*, CASRefPtr<Event>> m_Events;
};
}
