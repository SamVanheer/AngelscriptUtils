#include "AngelscriptUtils/event/EventSystem.h"

namespace asutils
{
EventSystem::EventSystem(EventRegistry& registry, const ReferencePointer<asIScriptContext>& context)
	: m_Registry(registry)
	, m_Context(context)
{
}

EventSystem::~EventSystem() = default;

void EventSystem::RemoveHandlersOfModule(asIScriptModule& module)
{
	for (auto event : m_Events)
	{
		event.second->RemoveHandlersOfModule(module);
	}
}

void EventSystem::RemoveAllHandlers()
{
	//Technically we could just clear the entire events map and achieve the same result
	for (auto event : m_Events)
	{
		event.second->RemoveAllHandlers();
	}
}

Event& EventSystem::InternalGetEvent(EventMetaData* metaData)
{
	auto it = m_Events.find(metaData);

	if (it != m_Events.end())
	{
		return *it->second.Get();
	}

	//Create on demand
	auto event = metaData->factory(*metaData, m_Context.Get());

	auto result = m_Events.emplace(metaData, std::move(event));

	return *result.first->second.Get();
}
}
