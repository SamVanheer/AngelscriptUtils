#pragma once

#include <type_traits>
#include <unordered_map>
#include <vector>

#include <angelscript.h>

#include "AngelscriptUtils/event/EventArgs.h"
#include "AngelscriptUtils/event/EventDispatcher.h"

#include "AngelscriptUtils/execution/Metadata.h"

#include "AngelscriptUtils/utility/GeneralUtils.h"
#include "AngelscriptUtils/utility/SmartPointers.h"
#include"AngelscriptUtils/utility/TypeStringUtils.h"

namespace asutils
{
/**
*	@brief Interface to an object that provides a list of event handlers
*/
class IEventHandlers
{
public:
	virtual ~IEventHandlers() = 0 {}

	/**
	*	@brief Returns the current number of event handlers
	*	You must call this every iteration because handlers can be added and removed during execution
	*/
	virtual asUINT GetCount() const = 0;

	virtual asIScriptFunction* GetFunction(asUINT index) const = 0;
};

using EventHandlers = std::vector<ReferencePointer<asIScriptFunction>>;

struct EventMetaData final : public IEventHandlers
{
	EventMetaData(ReferencePointer<const asITypeInfo>&& eventType)
		: EventType(std::move(eventType))
	{
	}

	const ReferencePointer<const asITypeInfo> EventType;

	EventHandlers EventHandlers;

	asUINT GetCount() const override final
	{
		return EventHandlers.size();
	}

	asIScriptFunction* GetFunction(asUINT index) const override final
	{
		if (index < EventHandlers.size())
		{
			return EventHandlers[index].Get();
		}

		return nullptr;
	}
};

/**
*	@brief Provides a way to register and look up event types
*	Can be used across library boundaries
*/
class IEventSystem
{
public:
	virtual ~IEventSystem() = 0 {}

	/**
	*	@brief Gets the context used by the event system
	*/
	virtual asIScriptContext* GetContext() const = 0;

	/**
	*	@brief Registers a type as an event type
	*	@param typeInfo The type to register
	*	@tparam T Native type that will be used for this event. Must provide an ObjectType specialization that matches the script type
	*/
	template<typename T, std::enable_if_t<std::is_base_of_v<EventArgs, T>, int> = 0>
	void Register(const asITypeInfo& typeInfo)
	{
		const ObjectType<T> type;

		//Verify that the native type is intended to be used with the given type
		if (strcmp(GetTypeNamespace(typeInfo), type.GetNamespace()) ||
			strcmp(typeInfo.GetName(), type.GetName()))
		{
			throw std::invalid_argument("Native and script types do not match");
		}

		InternalRegister(typeInfo);
	}

	/**
	*	@brief Gets an event dispatcher for the given type
	*	@exception std::invalid_argument If the event type has not been registered
	*/
	template<typename T>
	EventDispatcher<T> GetEvent()
	{
		const ObjectType<T> type;

		auto metaData = LookupEventHandlers(type.GetNamespace(), type.GetName());

		if (!metaData)
		{
			throw std::invalid_argument("Event type has not been registered");
		}

		return EventDispatcher<T>(*metaData, ReferencePointer<asIScriptContext>(GetContext()));
	}

	/**
	*	@brief Removes all event handlers defined in the given module from all events
	*/
	virtual void RemoveHandlersOfModule(asIScriptModule& module) = 0;

protected:
	virtual void InternalRegister(const asITypeInfo& typeInfo) = 0;

	virtual IEventHandlers* LookupEventHandlers(const char* typeNamespace, const char* typeName) const = 0;
};

/**
*	@brief Tracks all event types, stores the list of handlers for each type and provides a means of dispatching events
*	@details This class is designed to work across library boundaries
*	The library that creates this class instance must also register the API
*	All other libraries must only access this class through the IEventSystem interface to register types and dispatch events
*	The library that registers a type should also be the only library that dispatches events of that type for safety reasons
*
*	Usage: Create a class that extends from the @see EventArgs or @see PreemptableEventArgs class
*	Define an ObjectType specialization for it
*	Register the object type using @see RegisterEventClass, then call Register with the native and script types as parameters
*
*	To dispatch an event, call GetEvent<YourNativeEventClassType>().Dispatch({<parameters>});
*	You can also use the non-const overload to pass an object whose state can be evaluated after the dispatch call (@see EventDispatcher<T>::Dispatch(T&))
*	This can be useful when an event has return values
*
*	
*/
class EventSystem final : public IEventSystem
{
public:
	EventSystem(ReferencePointer<asIScriptContext>&& context)
		: m_Context(context)
	{
	}

	asIScriptContext* GetContext() const override final
	{
		return m_Context.Get();
	}

	void RemoveHandlersOfModule(asIScriptModule& module) override final
	{
		for (const auto& pair : m_Events)
		{
			auto& eventHandlers = pair.second->EventHandlers;

			eventHandlers.erase(std::remove_if(eventHandlers.begin(), eventHandlers.end(), [&](const auto& function)
				{
					if (function->GetFuncType() == asFUNC_DELEGATE)
					{
						return function->GetDelegateFunction()->GetModule() == &module;
					}
					else
					{
						return function->GetModule() == &module;
					}
				}), eventHandlers.end());
		}
	}

	void RemoveAllHandlers()
	{
		for (const auto& pair : m_Events)
		{
			pair.second->EventHandlers.clear();
		}
	}

	//These methods should only ever be accessed by the EventLocator class in the library that creates this class and registers the API
	EventMetaData* Lookup(const std::string_view typeNamespace, const std::string_view typeName) const
	{
		const auto hash = HashCombine(typeNamespace, typeName);

		auto it = m_Events.find(hash);

		if (it != m_Events.end())
		{
			return it->second.get();
		}

		return nullptr;
	}

	EventMetaData* Lookup(const asITypeInfo& typeInfo) const
	{
		return Lookup(GetTypeNamespace(typeInfo), typeInfo.GetName());
	}

protected:
	void InternalRegister(const asITypeInfo& typeInfo) override final
	{
		const auto hash = HashCombine(std::string_view{GetTypeNamespace(typeInfo)}, std::string_view{typeInfo.GetName()});

		if (m_Events.find(hash) != m_Events.end())
		{
			throw std::invalid_argument("The given event type has already been registered");
		}

		m_Events.emplace(hash, std::make_unique<EventMetaData>(ReferencePointer<const asITypeInfo>{&typeInfo}));
	}

	IEventHandlers* LookupEventHandlers(const char* typeNamespace, const char* typeName) const override final
	{
		if (!typeNamespace || !typeName)
		{
			//Can't throw an exception here because this is a cross-library API
			return nullptr;
		}

		return Lookup(typeNamespace, typeName);
	}

private:
	const ReferencePointer<asIScriptContext> m_Context;

	//The hash of the combined namespace and name is used as the key to avoid allocating on event lookup
	std::unordered_map<std::size_t, std::unique_ptr<EventMetaData>> m_Events;
};
}
