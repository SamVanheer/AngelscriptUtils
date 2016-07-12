#ifndef ANGELSCRIPT_CASHOOKMANAGER_H
#define ANGELSCRIPT_CASHOOKMANAGER_H

#include <vector>

#include <angelscript.h>

#include "CASHook.h"

class CASManager;
class CASModule;

class CASHookManager final
{
private:
	typedef std::vector<CASHook*> Hooks_t;

public:
	CASHookManager( CASManager& manager );
	~CASHookManager() = default;

	CASHook* FindHookByID( const as::HookID_t hookID ) const;

	bool AddHook( CASHook* pHook );

	void RegisterHooks( asIScriptEngine& engine );

	bool HookFunction( const as::HookID_t hookID, void* pValue, const int iTypeId );

	void UnhookFunction( const as::HookID_t hookID, asIScriptGeneric* pArguments );

	void UnhookModuleFunctions( CASModule* pModule );

	void DumpHookedFunctions() const;

private:
	bool ValidateHookFunction( const CASHook* pHook, const int iTypeId, void* pObject, const char* const pszScope, asIScriptFunction*& pOutFunction ) const;

private:
	CASManager& m_Manager;

	Hooks_t m_Hooks;

private:
	CASHookManager( const CASHookManager& ) = delete;
	CASHookManager& operator=( const CASHookManager& ) = delete;
};

#endif //ANGELSCRIPT_CASHOOKMANAGER_H