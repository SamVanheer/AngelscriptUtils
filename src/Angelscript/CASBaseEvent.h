#ifndef ANGELSCRIPT_CASBASEEVENT_H
#define ANGELSCRIPT_CASBASEEVENT_H

#include <vector>

#include <angelscript.h>

#include "util/ASUtil.h"

#include "wrapper/ASCallable.h"

class CASModule;

/**
*	@defgroup ASEvents Angelscript Events
*
*	@{
*/

/**
*	Stop modes for events. Allows you to specify whether events should continue executing after a function has handled it.
*/
enum class EventStopMode
{
	/**
	*	Call all functions no matter what.
	*/
	CALL_ALL,

	/**
	*	If any function in a module has handled the event, stop after executing the last function in that module.
	*/
	MODULE_HANDLED,

	/**
	*	Stop as soon as a function has handled it.
	*/
	ON_HANDLED
};

/**
*	Return codes for functions that hook into an event.
*/
enum class HookReturnCode
{
	/**
	*	Continue executing.
	*/
	CONTINUE,

	/**
	*	The function handled the event, stop.
	*/
	HANDLED
};

/**
*	Result codes for hook invocation.
*/
enum class HookCallResult
{
	/**
	*	An error occurred while executing the hook.
	*/
	FAILED,

	/**
	*	No functions handled the hook.
	*/
	NONE_HANDLED,

	/**
	*	One or more functions handled the hook.
	*/
	HANDLED
};

/**
*	Represents an event that can be triggered, and that scripts can hook into to be notified when it is triggered.
*/
class CASBaseEvent
{
private:
	typedef std::vector<asIScriptFunction*> Functions_t;

public:
	/**
	*	Constructor.
	*	@param accessMask Access mask. Which module types this hook is available to.
	*	@param stopMode Stop mode.
	*	@see EventStopMode
	*/
	CASBaseEvent( const asDWORD accessMask, const EventStopMode stopMode );
	~CASBaseEvent();

	/**
	*	@return Access mask.
	*/
	asDWORD GetAccessMask() const { return m_AccessMask; }

	/**
	*	@return Stop mode.
	*/
	EventStopMode GetStopMode() const { return m_StopMode; }

	/**
	*	@return The funcdef that represents this event.
	*/
	asIScriptFunction* GetFuncDef() const { return m_pFuncDef; }

	/**
	*	Sets the funcdef that represents this event.
	*	@param pFuncDef Funcdef.
	*/
	void SetFuncDef( asIScriptFunction* pFuncDef )
	{
		m_pFuncDef = pFuncDef;
	}

	/**
	*	@return Number of hooked functions.
	*/
	size_t GetFunctionCount() const;

	/**
	*	Gets a hooked function by index.
	*	@param uiIndex Index.
	*	@return Function.
	*/
	asIScriptFunction* GetFunctionByIndex( const size_t uiIndex ) const;

	/**
	*	Adds a new function. Cannot be called while this event is being called.
	*	Warning: if the function does not match the event parameters and return type, this will cause problems.
	*	@param pFunction Function to add.
	*	@return true if the function was either added or already added before, false otherwise.
	*/
	bool AddFunction( asIScriptFunction* pFunction );

	/**
	*	Hooks a function to an event.
	*	Used by scripts only.
	*	@param pValue Function pointer.
	*	@param iTypeId Function pointer type id.
	*	@return true on success, false otherwise.
	*/
	bool Hook( void* pValue, const int iTypeId );

	/**
	*	Removes a function. Cannot be called while this event is being called.
	*	@param pFunction Function to remove.
	*/
	void RemoveFunction( asIScriptFunction* pFunction );

	/**
	*	Unhooks a function from an event.
	*	Used by scripts only.
	*	@param pValue Function pointer.
	*	@param iTypeId Function pointer type id.
	*/
	void Unhook( void* pValue, const int iTypeId );

	/**
	*	Removes all of the functions that belong to the given module.
	*/
	void RemoveFunctionsOfModule( CASModule* pModule );

	/**
	*	Removes all functions.
	*/
	void RemoveAllFunctions();

private:
	/**
	*	Validates the given hook function.
	*/
	bool ValidateHookFunction( const int iTypeId, void* pObject, const char* const pszScope, asIScriptFunction*& pOutFunction ) const;

public:
	/**
	*	Calls the given function using the given context.
	*	@param pContext Context to use.
	*	@param flags Flags.
	*	@param list Argument list.
	*/
	HookCallResult VCall( asIScriptContext* pContext, CallFlags_t flags, va_list list );

	/**
	*	@see VCall( asIScriptContext* pContext, CallFlags_t flags, va_list list )
	*/
	HookCallResult VCall( asIScriptContext* pContext, va_list list );

	/**
	*	Acquires a context using asIScriptEngine::RequestContext
	*	@see VCall( asIScriptContext* pContext, CallFlags_t flags, va_list list )
	*/
	HookCallResult VCall( CallFlags_t flags, va_list list );

	/**
	*	Calls the given function using the given context.
	*	@param pContext Context to use.
	*	@param flags Flags.
	*	@param ... Arguments.
	*/
	HookCallResult Call( asIScriptContext* pContext, CallFlags_t flags, ... );

	/**
	*	@see Call( asIScriptContext* pContext, CallFlags_t flags, ... )
	*/
	HookCallResult Call( asIScriptContext* pContext, ... );

	/**
	*	Acquires a context using asIScriptEngine::RequestContext
	*	@see Call( asIScriptContext* pContext, CallFlags_t flags, ... )
	*/
	HookCallResult Call( CallFlags_t flags, ... );

	/**
	*	Dumps all hooked functions to stdout.
	*	@param pszName Optional. Name to print.
	*/
	void DumpHookedFunctions( const char* const pszName ) const;

private:
	const asDWORD m_AccessMask;
	const EventStopMode m_StopMode;

	asIScriptFunction* m_pFuncDef = nullptr;

	Functions_t m_Functions;

	//Used to prevent adding/removing hooks while invoking the hook in question.
	int m_iInCallCount = 0;

private:
	CASBaseEvent( const CASBaseEvent& ) = delete;
	CASBaseEvent& operator=( const CASBaseEvent& ) = delete;
};

/**
*	Registers the HookReturnCode enum.
*	@param engine Script engine.
*/
void RegisterScriptHookReturnCode( asIScriptEngine& engine );

/**
*	Registers CBaseEvent class members for pszObjectName. Also registers casts to and from pszObjectName if it differs from CBaseEvent.
*	@param engine Script engine.
*	@param pszObjectName Name of the class that is being registered.
*/
template<typename CLASS>
void RegisterScriptCBaseEvent( asIScriptEngine& engine, const char* const pszObjectName )
{
	as::RegisterCasts<CASBaseEvent, CLASS>( engine, "CBaseEvent", pszObjectName, &as::Cast_UpCast, &as::Cast_DownCast );

	engine.RegisterObjectMethod(
		pszObjectName, "bool Hook(?& in pFunction)",
		asMETHOD( CLASS, Hook ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void Unhook(?& in pFunction)",
		asMETHOD( CLASS, Unhook ), asCALL_THISCALL );
}

/**
*	Registers the CBaseEvent class.
*	@param engine Script engine.
*/
void RegisterScriptCBaseEvent( asIScriptEngine& engine );

/** @} */

#endif //ANGELSCRIPT_CASBASEEVENT_H
