#ifndef ANGELSCRIPT_CASHOOK_H
#define ANGELSCRIPT_CASHOOK_H

#include <cstdarg>
#include <cstdint>
#include <limits>
#include <vector>

#include <angelscript.h>

#include "wrapper/ASCallable.h"

class CASModule;

enum class HookStopMode
{
	CALL_ALL,
	MODULE_HANDLED,
	ON_HANDLED
};

enum class HookReturnCode
{
	CONTINUE,
	HANDLED
};

enum class HookCallResult
{
	FAILED,
	NONE_HANDLED,
	HANDLED
};

namespace as
{
typedef uint32_t HookID_t;

const HookID_t INVALID_HOOK_ID = 0;

const HookID_t FIRST_HOOK_ID = 1;

const HookID_t LAST_HOOK_ID = std::numeric_limits<HookID_t>::max();
}

class CASHook final
{
private:
	typedef std::vector<asIScriptFunction*> Functions_t;

public:
	CASHook( const char* const pszName, const char* pszArguments, const char* const pszCategory, const asDWORD accessMask, const HookStopMode stopMode );

	const char* GetName() const { return m_pszName; }

	const char* GetArguments() const { return m_pszArguments; }

	const char* GetCategory() const { return m_pszCategory; }

	asDWORD GetAccessMask() const { return m_AccessMask; }

	HookStopMode GetStopMode() const { return m_StopMode; }

	as::HookID_t GetHookID() const { return m_HookID; }

	as::HookID_t& GetMutableHookID() { return m_HookID; }

	void SetHookID( const as::HookID_t hookID )
	{
		m_HookID = hookID;
	}

	asIScriptFunction* GetFuncDef() const { return m_pFuncDef; }

	void SetFuncDef( asIScriptFunction* pFuncDef )
	{
		m_pFuncDef = pFuncDef;
	}

	size_t GetFunctionCount() const;

	asIScriptFunction* GetFunctionByIndex( const size_t uiIndex ) const;

	bool AddFunction( asIScriptFunction* pFunction );

	void RemoveFunction( asIScriptFunction* pFunction );

	void RemoveFunctionsOfModule( CASModule* pModule );

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

#endif //ANGELSCRIPT_CASHOOK_H