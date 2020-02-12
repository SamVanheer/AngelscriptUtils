#include "AngelscriptUtils/event/EventSystem.h"

namespace asutils
{
EventSystem::EventSystem(EventRegistry& registry, const CASRefPtr<asIScriptContext>& context)
	: m_Registry(registry)
	, m_Context(context)
{
}

EventSystem::~EventSystem() = default;

void EventSystem::RemoveFunctionsOfModule(asIScriptModule& module)
{
	for (auto event : m_Events)
	{
		event.second->RemoveFunctionsOfModule(module);
	}
}

Event& EventSystem::InternalGetEvent(EventMetaData* metaData)
{
	if (metaData == nullptr)
	{
		throw std::invalid_argument("The type T is an unregistered event type");
	}

	auto it = m_Events.find(metaData);

	if (it != m_Events.end())
	{
		return *it->second.Get();
	}

	//Create on demand
	auto event = metaData->factory(m_Context);

	auto result = m_Events.emplace(metaData, std::move(event));

	return *result.first->second.Get();
}
}
