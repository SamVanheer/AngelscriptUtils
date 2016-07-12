#ifndef ANGELSCRIPT_CASMANAGER_H
#define ANGELSCRIPT_CASMANAGER_H

#include "CASModuleManager.h"
#include "CASHookManager.h"

class asIScriptEngine;
struct asSMessageInfo;

/**
*	Manages the Angelscript engine instance, the module and hook managers.
*/
class CASManager final
{
public:
	/**
	*	Constructor.
	*/
	CASManager();

	/**
	*	Destructor.
	*/
	~CASManager();

	/**
	*	@return The script engine.
	*/
	asIScriptEngine* GetEngine() { return m_pScriptEngine; }

	/**
	*	@return The module manager.
	*/
	CASModuleManager& GetModuleManager() { return m_ModuleManager; }

	/**
	*	@return The hook manager.
	*/
	CASHookManager& GetHookManager() { return m_HookManager; }

	/**
	*	Initializes the manager.
	*	@return true on success, false otherwise.
	*/
	bool Initialize();

	/**
	*	Shuts down the manager.
	*/
	void Shutdown();

private:
	/**
	*	@see asIScriptEngine::SetMessageCallback
	*/
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