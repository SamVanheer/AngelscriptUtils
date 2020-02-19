#ifndef ANGELSCRIPT_CASMODULEMANAGER_H
#define ANGELSCRIPT_CASMODULEMANAGER_H

#include <memory>
#include <string>
#include <utility>
#include <unordered_map>
#include <vector>

#include <angelscript.h>

class CASModule;
class IASModuleBuilder;

/**
*	@addtogroup ASModule
*
*	@{
*/

/**
*	Manages a list of module descriptors and modules.
*/
class CASModuleManager final
{
public:
	/**
	*	Constructor.
	*	@param engine Script engine.
	*/
	CASModuleManager( asIScriptEngine& engine );

	/**
	*	Destructor.
	*/
	~CASModuleManager();

	/**
	*	@return The script engine.
	*/
	asIScriptEngine& GetEngine() { return m_Engine; }

	/**
	*	Builds a module
	*	@param pszModuleName Name of the module. Must be unique
	*	@param accessMask Module access mask
	*	@param builder Builder to use
	*	@return On successful build, the module. Otherwise, null
	*/
	CASModule* BuildModule( const char* const pszModuleName, const asDWORD accessMask, IASModuleBuilder& builder );

public:
	/**
	*	@return The number of modules that are currently loaded.
	*/
	size_t GetModuleCount() const;

	/**
	*	Finds a module by name.
	*	@param pszModuleName Name of the module. Case sensitive.
	*	@return Module, or null if the module couldn't be found.
	*/
	const CASModule* FindModuleByName( const char* const pszModuleName ) const;

	/**
	*	@copydoc FindModuleByName( const char* const pszModuleName ) const
	*/
	CASModule* FindModuleByName( const char* const pszModuleName );

	/**
	*	Finds a module by index.
	*	@param uiIndex Index of the module.
	*	@return Module, or null if the module couldn't be found.
	*/
	const CASModule* FindModuleByIndex( const size_t uiIndex ) const;

	/**
	*	@copydoc FindModuleByIndex( const size_t uiIndex ) const
	*/
	CASModule* FindModuleByIndex( const size_t uiIndex );

private:
	/**
	*	Adds a module.
	*	@param pModule Module to add.
	*	@return true if the module was added, false otherwise.
	*/
	bool AddModule( CASModule* pModule );

public:
	/**
	*	Removes a module. The module may not be destroyed immediately if there are active references held to it.
	*	@param pModule Module to remove.
	*/
	void RemoveModule( CASModule* pModule );

	/**
	*	@see RemoveModule( CASModule* pModule )
	*	@param pszModuleName Name of the module. Case sensitive.
	*/
	void RemoveModule( const char* const pszModuleName );

	/**
	*	Removes all modules and descriptors.
	*/
	void Clear();

private:
	asIScriptEngine& m_Engine;

	std::vector<CASModule*> m_Modules;

private:
	CASModuleManager( const CASModuleManager& ) = delete;
	CASModuleManager& operator=( const CASModuleManager& ) = delete;
};

/** @} */

#endif //ANGELSCRIPT_CASMODULEMANAGER_H