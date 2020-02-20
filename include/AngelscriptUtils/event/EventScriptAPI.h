#pragma once

#include <string>

#include "AngelscriptUtils/event/EventArgs.h"

#include "AngelscriptUtils/execution/Metadata.h"

#include "AngelscriptUtils/utility/BaseClasses.h"

class asIScriptEngine;

namespace asutils
{
/**
*	@brief Registers the EventSystem class
*	Should only be used if you intend to use a custom Event<TEvent> class constructor with custom a EventSystem type
*	@see RegisterEventAPI
*/
void RegisterEventSystem(asIScriptEngine& engine);

/**
*	@brief Registers the event API
*	
*	@details This includes the EventSystem class and the Event class
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
*	@param provideEventSystemInstance If true, the EventSystem class will be registered along with a constructor for Event<TEvent> that takes an EventSystem handle
*		If false, you will need to provide your own constructor that provides an EventSystem instance to the @see EventLocator class
*		This can be used to have a global EventSystem instance, or an instance stored as user data on an Angelscript object
*/
void RegisterEventAPI(asIScriptEngine& engine, const bool provideEventSystemInstance);

std::string FormatEventHandlerFuncdef(const char* className);

template<typename T, std::enable_if_t<std::is_base_of_v<PreemptableEventArgs, T>, int> = 0>
inline void RegisterPreemptableEventClass(asIScriptEngine& engine, const char* className)
{
	engine.RegisterObjectMethod(className, "bool get_Handled() const property", asMETHOD(T, IsHandled), asCALL_THISCALL);
	engine.RegisterObjectMethod(className, "void set_Handled(bool value) property", asMETHOD(T, SetHandled), asCALL_THISCALL);
}

template<typename T, std::enable_if_t<!std::is_base_of_v<PreemptableEventArgs, T>, int> = 0>
inline void RegisterPreemptableEventClass(asIScriptEngine&, const char*)
{
	//Nothing
}

/**
*	@brief Registers an event type as well as a funcdef for the handler
*	The native type must provide an ObjectType specialization
*	@tparam C++ type of the event
*	@param className Script type of the event
*/
template<typename T, std::enable_if_t<std::is_base_of_v<EventArgs, T>, int> = 0>
inline void RegisterEventClass(asIScriptEngine& engine)
{
	const ObjectType<T> type;

	const auto className = type.GetName();

	const std::string oldNamespace = engine.GetDefaultNamespace();

	engine.SetDefaultNamespace(type.GetNamespace());

	engine.RegisterObjectType(className, sizeof(T), type.GetFlags());

	RegisterReferenceCountedClass<T>(engine, className);

	//Also register a funcdef to make casting object methods easier
	engine.RegisterFuncdef((FormatEventHandlerFuncdef(className)).c_str());

	RegisterPreemptableEventClass<T>(engine, className);

	engine.SetDefaultNamespace(oldNamespace.c_str());
}
}
