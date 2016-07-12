#ifndef ANGELSCRIPT_CASMANAGER_H
#define ANGELSCRIPT_CASMANAGER_H

#include "CASModuleManager.h"
#include "CASHookManager.h"

class asIScriptEngine;
struct asSMessageInfo;

class CASManager final
{
public:
	CASManager();
	~CASManager() = default;

	asIScriptEngine* GetEngine() { return m_pScriptEngine; }

	CASModuleManager& GetModuleManager() { return m_ModuleManager; }

	CASHookManager& GetHookManager() { return m_HookManager; }

	bool Initialize();

	void Shutdown();

private:
	void MessageCallback( const asSMessageInfo* pMsg );

private:
	asIScriptEngine* m_pScriptEngine = nullptr;

	CASModuleManager m_ModuleManager;
	CASHookManager m_HookManager;

private:
	CASManager( const CASManager& ) = delete;
	CASManager& operator=( const CASManager& ) = delete;
};

#endif //ANGELSCRIPT_CASMANAGER_H