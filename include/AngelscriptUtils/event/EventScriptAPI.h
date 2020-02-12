#pragma once

#include <string>

#include "AngelscriptUtils/event/EventArgs.h"
#include "AngelscriptUtils/util/CASBaseClass.h"

class asIScriptEngine;

namespace asutils
{
/**
*	@brief Registers the event API
*	
*	This includes the EventSystem class and the Event class
*
*	Event subscription in scripts is handled by combining both:
*	\code{.unparsed}
*	Event<MyEvent>(scriptEventSystem).Subscribe(@MyEventHandler);
*   \endcode
*
*	Event unsubscription works the same way:
*	\code{.unparsed}
*	Event<MyEvent>(scriptEventSystem).Unsubscribe(@MyEventHandler);
*   \endcode
*
*	Repeated Subscribe calls with the same handler will be silently ignored
*/
void RegisterEventAPI(asIScriptEngine& engine);

std::string FormatEventHandlerFuncdef(const char* className);

/**
*	@brief Registers an event type as well as a funcdef for the handler
*	@tparam C++ type of the event
*	@param className Script type of the event
*/
template<typename T, std::enable_if_t<std::is_base_of<EventArgs, T>::value, int> = 0>
inline void RegisterEventClass(asIScriptEngine& engine, const char* className)
{
	engine.RegisterObjectType(className, sizeof(T), asOBJ_REF);

	as::RegisterRefCountedBaseClass<T>(&engine, className);

	//Also register a funcdef to make casting object methods easier
	engine.RegisterFuncdef((FormatEventHandlerFuncdef(className)).c_str());
}
}
