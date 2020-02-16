#ifndef ANGELSCRIPT_IASINITIALIZER_H
#define ANGELSCRIPT_IASINITIALIZER_H

#include <angelscript.h>

/**
*	@addtogroup ASManager
*
*	@{
*/

class CASManager;

/**
*	Used by the manager to initialize itself.
*/
class IASInitializer
{
public:
	virtual ~IASInitializer() = 0;

	/**
	*	Called when initialization has started.
	*/
	virtual void OnInitBegin() {}

	/**
	*	Called when initialization has ended.
	*	@param bSuccess Whether initialization succeeded.
	*/
	virtual void OnInitEnd( const bool bSuccess );

	/**
	*	Allows applications to provide a message callback to be installed upon initialization.
	*	@param[ out ] outFuncPtr Function to use.
	*	@param[ out ] pOutObj Object to use.
	*	@param[ out ] outCallConv Calling convention.
	*	@return true if a callback was set, false otherwise.
	*/
	virtual bool GetMessageCallback( asSFuncPtr& outFuncPtr, void*& pOutObj, asDWORD& outCallConv );

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
	*	Should register the remainder of the API.
	*	@param manager Manager.
	*	@return true on success, false otherwise.
	*/
	virtual bool RegisterAPI( CASManager& manager ) = 0;
};

inline IASInitializer::~IASInitializer()
{
}

inline void IASInitializer::OnInitEnd( const bool )
{
}

inline bool IASInitializer::GetMessageCallback( asSFuncPtr&, void*&, asDWORD& )
{
	return false;
}

/** @} */

#endif //ANGELSCRIPT_IASINITIALIZER_H