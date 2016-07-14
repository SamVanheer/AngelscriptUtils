#ifndef ANGELSCRIPT_CASHOOKMANAGER_H
#define ANGELSCRIPT_CASHOOKMANAGER_H

#include <vector>

#include <angelscript.h>

#include "CASHook.h"

class CASManager;
class CASModule;

/**
*	@addtogroup ASHooks
*
*	@{
*/

/**
*	Manages the list of hooks.
*/
class CASHookManager final
{
private:
	typedef std::vector<CASHook*> Hooks_t;

public:
	/**
	*	Constructor.
	*	@param manager Manager.
	*/
	CASHookManager( CASManager& manager );

	/**
	*	Destructor.
	*/
	~CASHookManager();

	/**
	*	Finds a hook by ID.
	*	@param hookID Hook ID.
	*	@return The hook, or null if it couldn't be found.
	*/
	CASHook* FindHookByID( const as::HookID_t hookID ) const;

	/**
	*	Adds a hook.
	*	@param pHook Hook to add.
	*	@return true if the hook was added, false otherwise.
	*/
	bool AddHook( CASHook* pHook );

	/**
	*	Registers required types, this class and all hooks.
	*/
	void RegisterHooks( asIScriptEngine& engine );

	/**
	*	Hooks a function to a hook.
	*	Used by scripts only.
	*	@param hookID Hook ID.
	*	@param pValue Function pointer.
	*	@param iTypeId Function pointer type id.
	*	@return true on success, false otherwise.
	*/
	bool HookFunction( const as::HookID_t hookID, void* pValue, const int iTypeId );

	/**
	*	Unhooks a function from a hook.
	*	Used by scripts only.
	*	@param hookID Hook ID.
	*	@param pValue Function pointer.
	*	@param iTypeId Function pointer type id.
	*/
	void UnhookFunction( const as::HookID_t hookID, void* pValue, const int iTypeId );

	/**
	*	Unhooks all functions that are part of the given module.
	*	@param pModule Module.
	*/
	void UnhookModuleFunctions( CASModule* pModule );

	/**
	*	Unhooks all functions.
	*/
	void UnhookAllFunctions();

	/**
	*	Dumps all hooked functions to stdout.
	*/
	void DumpHookedFunctions() const;

private:
	/**
	*	Validates the given hook function.
	*/
	bool ValidateHookFunction( const CASHook* pHook, const int iTypeId, void* pObject, const char* const pszScope, asIScriptFunction*& pOutFunction ) const;

private:
	CASManager& m_Manager;

	Hooks_t m_Hooks;

private:
	CASHookManager( const CASHookManager& ) = delete;
	CASHookManager& operator=( const CASHookManager& ) = delete;
};

/** @} */

#endif //ANGELSCRIPT_CASHOOKMANAGER_H