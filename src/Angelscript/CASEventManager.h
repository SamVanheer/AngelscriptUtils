#ifndef ANGELSCRIPT_CASEVENTMANAGER_H
#define ANGELSCRIPT_CASEVENTMANAGER_H

#include <cstdint>
#include <string>
#include <vector>

#include <angelscript.h>

#include "CASEvent.h"

class CASManager;
class CASModule;

/**
*	@addtogroup ASEvents
*
*	@{
*/

/**
*	Manages the list of events.
*	Can store a maximum of UINT32_MAX events.
*/
class CASEventManager final
{
private:
	typedef std::vector<CASEvent*> Events_t;

public:
	/**
	*	Constructor.
	*	@param manager Manager.
	*/
	CASEventManager( CASManager& manager );

	/**
	*	Destructor.
	*/
	~CASEventManager();

	/**
	*	@return The number of events.
	*/
	uint32_t GetEventCount() const { return static_cast<uint32_t>( m_Events.size() ); }

	/**
	*	Gets an event by index.
	*	@param uiIndex Event index. Must be smaller than GetEventCount().
	*	@return If the index is valid, the event. Otherwise, null.
	*/
	CASEvent* GetEventByIndex( const uint32_t uiIndex );

	/**
	*	Finds an event by its name. The given name must specify its category if it has one.
	*	Format: \<Category\>::\<Name\>
	*	May optionally specify Events:: as the start of the category.
	*	@return If found, the event. Otherwise, null.
	*/
	CASEvent* FindEventByName( const std::string& szName );

	/**
	*	Adds an event.
	*	@param pEvent Event to add.
	*	@return true if the event was added, false otherwise.
	*/
	bool AddEvent( CASEvent* pEvent );

	/**
	*	Registers this class instance and all events.
	*/
	void RegisterEvents( asIScriptEngine& engine );

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
	CASManager& m_Manager;

	Events_t m_Events;

private:
	CASEventManager( const CASEventManager& ) = delete;
	CASEventManager& operator=( const CASEventManager& ) = delete;
};

/**
*	Registers the event API.
*	@param engine Script engine.
*/
void RegisterScriptEventAPI( asIScriptEngine& engine );

/** @} */

#endif //ANGELSCRIPT_CASEVENTMANAGER_H
