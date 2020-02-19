#pragma once

#include <functional>
#include <memory>
#include <typeinfo>
#include <unordered_map>

#include <angelscript.h>

#include "AngelscriptUtils/event/Event.h"
#include "AngelscriptUtils/event/EventArgs.h"

#include "AngelscriptUtils/utility/SmartPointers.h"

namespace asutils
{
/**
*	@brief Contains the type info and a factory to create instances of the event class for an event argument type
*/
struct EventMetaData final
{
	const asITypeInfo& type;

	const std::function<ReferencePointer<Event>(const EventMetaData&, asIScriptContext*)> factory;

	EventMetaData(const asITypeInfo& type, std::function<ReferencePointer<Event>(const EventMetaData&, asIScriptContext*)> factory)
		: type(type)
		, factory(factory)
	{
		type.AddRef();
	}

	~EventMetaData()
	{
		type.Release();
	}
};

/**
*	@brief Stores metadata about all event types exposed to scripts and provides a means of instantiating them
*/
class EventRegistry final
{
private:
	using TypeInfoRef = std::reference_wrapper<const std::type_info>;

	struct Hasher
	{
		std::size_t operator()(TypeInfoRef code) const
		{
			return code.get().hash_code();
		}
	};

	struct EqualTo
	{
		bool operator()(TypeInfoRef lhs, TypeInfoRef rhs) const
		{
			return lhs.get() == rhs.get();
		}
	};

public:
	EventRegistry() = default;
	~EventRegistry() = default;

	EventRegistry(const EventRegistry&) = delete;
	EventRegistry& operator=(const EventRegistry&) = delete;

	/**
	*	@brief Registers a C++ class that derives from \see asutils::EventArgs and binds it to the given script type
	*/
	template<typename T, std::enable_if_t<std::is_base_of<EventArgs, T>::value, int> = 0>
	void Register(const asITypeInfo& type)
	{
		auto data = std::make_unique<EventMetaData>(type, [](const EventMetaData& metaData, asIScriptContext* context) -> ReferencePointer<Event>
			{
				return ReferencePointer<Event>(new TypedEvent<T>(metaData, ReferencePointer<asIScriptContext>(context)));
			});

		m_ASTypeToData[&type] = data.get();

		m_CppTypeToData[typeid(T)] = std::move(data);
	}

	/**
	*	@brief Looks up event metadata by C++ type
	*	Should not be used directly
	*/
	template<typename T, std::enable_if_t<std::is_base_of<EventArgs, T>::value, int> = 0>
	EventMetaData* Lookup()
	{
		auto it = m_CppTypeToData.find(typeid(T));

		if (it != m_CppTypeToData.end())
		{
			return it->second.get();
		}

		return nullptr;
	}

	/**
	*	@brief Looks up event metadata by script type
	*	Should not be used directly
	*/
	EventMetaData* Lookup(const asITypeInfo& type)
	{
		auto it = m_ASTypeToData.find(&type);

		if (it != m_ASTypeToData.end())
		{
			return it->second;
		}

		return nullptr;
	}

private:
	std::unordered_map<TypeInfoRef, std::unique_ptr<EventMetaData>, Hasher, EqualTo> m_CppTypeToData;
	std::unordered_map<const asITypeInfo*, EventMetaData*> m_ASTypeToData;
};
}
