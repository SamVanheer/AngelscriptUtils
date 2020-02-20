#pragma once

#include <angelscript.h>

#include "AngelscriptUtils/utility/SmartPointers.h"

namespace asutils
{
struct EventMetaData;
class EventSystem;

/**
*	@brief Angelscript template class used to locate events so scripts can subscribe and unsubscribe
*/
class EventLocator final
{
public:
	EventLocator(asITypeInfo* type, EventSystem* eventSystem);

	EventLocator(const EventLocator&) = delete;
	EventLocator& operator=(const EventLocator&) = delete;

	bool IsSubscribed(void* object, int typeId) const;

	bool IsSubscribed(asIScriptFunction& function) const;

	void Subscribe(void* object, int typeId);

	void Unsubscribe(void* object, int typeId);

private:
	asIScriptFunction* GetFunctionFromVariable(void* object, int typeId) const;

	bool ValidateFunctionFormat(asIScriptFunction& function) const;

private:
	const ReferencePointer<const asIScriptEngine> m_Engine;
	EventMetaData* m_EventData;
};
}
