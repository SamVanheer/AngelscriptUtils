#ifndef ANGELSCRIPT_IASINITIALIZER_H
#define ANGELSCRIPT_IASINITIALIZER_H

#include <angelscript.h>

/**
*	@addtogroup ASManager
*
*	@{
*/

class CASManager;
class CASEventManager;

/**
*	Used by the manager to initialize itself.
*/
class IASInitializer
{
public:
	virtual ~IASInitializer() = 0;

	/**
	*	Allows applications to provide a message callback to be installed upon initialization.
	*	@param[ out ] outFuncPtr Function to use.
	*	@param[ out ] pOutObj Object to use.
	*	@param[ out ] outCallConv Calling convention.
	*	@return true if a callback was set, false otherwise.
	*/
	virtual bool GetMessageCallback( asSFuncPtr& outFuncPtr, void*& pOutObj, asDWORD& outCallConv ) { return false; }

	/**
	*	@return Whether to create an event manager.
	*/
	virtual bool UseEventManager() { return false; }

	/**
	*	Gets the namespace in which events should be registered. Default "Events".
	*/
	virtual const char* GetEventNamespace() { return "Events"; }

	/**
	*	Should register the core API, including the following types:
	*	string
	*	dictionary
	*	array
	*	@param manager Manager.
	*	@return true on success, false otherwise.
	*/
	virtual bool RegisterCoreAPI( CASManager& manager ) = 0;

	/**
	*	Should register events.
	*	@param manager Manager.
	*	@param eventManager Event manager.
	*	@return true on success, false otherwise.
	*/
	virtual bool AddEvents( CASManager& manager, CASEventManager& eventManager ) { return true; }

	/**
	*	Should register the remainder of the API.
	*	@param manager Manager.
	*	@return true on success, false otherwise.
	*/
	virtual bool RegisterAPI( CASManager& manager ) = 0;
};

inline IASInitializer::~IASInitializer()
{
}

/** @} */

#endif //ANGELSCRIPT_IASINITIALIZER_H