#ifndef ANGELSCRIPT_CASEVENTMANAGER_H
#define ANGELSCRIPT_CASEVENTMANAGER_H

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
	*	Finds an event by its name. The given name must specify its category if it has one.
	*	Format: <Category>::<Name>
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
	*	Registers required types, this class and all events.
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
