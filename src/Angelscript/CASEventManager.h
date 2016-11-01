#ifndef ANGELSCRIPT_CASEVENTMANAGER_H
#define ANGELSCRIPT_CASEVENTMANAGER_H

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
	*	Finds an event by ID.
	*	@param eventID Event ID.
	*	@return The event, or null if it couldn't be found.
	*/
	CASEvent* FindEventByID( const as::EventID_t eventID ) const;

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
	*	Hooks a function to an event.
	*	Used by scripts only.
	*	@param eventID Event ID.
	*	@param pValue Function pointer.
	*	@param iTypeId Function pointer type id.
	*	@return true on success, false otherwise.
	*/
	bool HookFunction( const as::EventID_t eventID, void* pValue, const int iTypeId );

	/**
	*	Unhooks a function from an event.
	*	Used by scripts only.
	*	@param eventID Event ID.
	*	@param pValue Function pointer.
	*	@param iTypeId Function pointer type id.
	*/
	void UnhookFunction( const as::EventID_t eventID, void* pValue, const int iTypeId );

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
	bool ValidateHookFunction( const CASEvent* pEvent, const int iTypeId, void* pObject, const char* const pszScope, asIScriptFunction*& pOutFunction ) const;

private:
	CASManager& m_Manager;

	Events_t m_Events;

private:
	CASEventManager( const CASEventManager& ) = delete;
	CASEventManager& operator=( const CASEventManager& ) = delete;
};

/** @} */

#endif //ANGELSCRIPT_CASEVENTMANAGER_H
