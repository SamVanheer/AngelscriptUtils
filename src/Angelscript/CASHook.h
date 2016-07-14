#ifndef ANGELSCRIPT_CASHOOK_H
#define ANGELSCRIPT_CASHOOK_H

#include <cstdarg>
#include <cstdint>
#include <limits>
#include <vector>

#include <angelscript.h>

#include "wrapper/ASCallable.h"

class CASModule;

/**
*	@defgroup ASHooks Angelscript Hooks
*
*	@{
*/

/**
*	Stop modes for hooks. Allows you to specify whether hooks should continue executing after a function has handled it.
*/
enum class HookStopMode
{
	/**
	*	Call all functions no matter what.
	*/
	CALL_ALL,

	/**
	*	If any function in a module has handled the hook, stop after executing the last function in that module.
	*/
	MODULE_HANDLED,

	/**
	*	Stop as soon as a function has handled it.
	*/
	ON_HANDLED
};

/**
*	Return codes for functions that hook into a hook.
*/
enum class HookReturnCode
{
	/**
	*	Continue executing.
	*/
	CONTINUE,

	/**
	*	The function handled the hook, stop.
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

namespace as
{
/**
*	Type used for hook IDs.
*/
typedef uint32_t HookID_t;

/**
*	Invalid hook identifier.
*/
const HookID_t INVALID_HOOK_ID = 0;

/**
*	First valid hook ID.
*/
const HookID_t FIRST_HOOK_ID = 1;

/**
*	Last valid hook ID.
*/
const HookID_t LAST_HOOK_ID = std::numeric_limits<HookID_t>::max();
}

/**
*	Represents an event that script functions can hook into.
*/
class CASHook final
{
private:
	typedef std::vector<asIScriptFunction*> Functions_t;

public:
	/**
	*	Constructor.
	*	@param pszName Name of this hook.
	*	@param pszArguments The arguments passed to hooked functions. This is a comma delimited list of argument types, e.g. "const string& in, bool& out".
	*	@param pszCategory Which category this hook is in. This is a double colon delimited list, e.g. "Game::Player".
	*	@param accessMask Access mask. Which module types this hook is available to.
	*	@param stopMode Stop mode.
	*	@see HookStopMode
	*/
	CASHook( const char* const pszName, const char* pszArguments, const char* const pszCategory, const asDWORD accessMask, const HookStopMode stopMode );

	/**
	*	@return Hook name.
	*/
	const char* GetName() const { return m_pszName; }

	/**
	*	@return Hook arguments.
	*/
	const char* GetArguments() const { return m_pszArguments; }

	/**
	*	@return Hook category.
	*/
	const char* GetCategory() const { return m_pszCategory; }

	/**
	*	@return Access mask.
	*/
	asDWORD GetAccessMask() const { return m_AccessMask; }

	/**
	*	@return Stop mode.
	*/
	HookStopMode GetStopMode() const { return m_StopMode; }

	/**
	*	@return Hook ID.
	*/
	as::HookID_t GetHookID() const { return m_HookID; }

	/**
	*	@return Mutable hook ID. Only used by the hook manager.
	*/
	as::HookID_t& GetMutableHookID() { return m_HookID; }

	/**
	*	Sets the hook ID. Only used by the hook manager.
	*	@param hookID Hook ID.
	*/
	void SetHookID( const as::HookID_t hookID )
	{
		m_HookID = hookID;
	}

	/**
	*	@return The funcdef that represents this hook.
	*/
	asIScriptFunction* GetFuncDef() const { return m_pFuncDef; }

	/**
	*	Sets the funcdef that represents this hook.
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
	*	Adds a new function. Cannot be called while this hook is being called.
	*	Warning: if the function does not match the hook parameters and return type, this will cause problems.
	*	@param pFunction Function to add.
	*	@return true if the function was either added or already added before, false otherwise.
	*/
	bool AddFunction( asIScriptFunction* pFunction );

	/**
	*	Removes a function. Cannot be called while this hook is being called.
	*	@param pFunction Function to remove.
	*/
	void RemoveFunction( asIScriptFunction* pFunction );

	/**
	*	Removes all of the functions that belong to the given module.
	*/
	void RemoveFunctionsOfModule( CASModule* pModule );

	/**
	*	Removes all functions.
	*/
	void RemoveAllFunctions();

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

private:
	const char* const m_pszName;
	const char* const m_pszArguments;
	const char* const m_pszCategory;
	const asDWORD m_AccessMask;
	const HookStopMode m_StopMode;

	as::HookID_t m_HookID = as::INVALID_HOOK_ID;

	asIScriptFunction* m_pFuncDef = nullptr;

	Functions_t m_Functions;

	//Used to prevent adding/removing hooks while invoking the hook in question.
	int m_iInCallCount = 0;

private:
	CASHook( const CASHook& ) = delete;
	CASHook& operator=( const CASHook& ) = delete;
};

/** @} */

#endif //ANGELSCRIPT_CASHOOK_H