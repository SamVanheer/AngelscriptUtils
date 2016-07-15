#ifndef ANGELSCRIPT_IASINITIALIZER_H
#define ANGELSCRIPT_IASINITIALIZER_H

/**
*	@addtogroup ASManager
*
*	@{
*/

class CASManager;
class CASHookManager;

/**
*	Used by the manager to initialize itself.
*/
class IASInitializer
{
public:
	virtual ~IASInitializer() = 0;

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
	*	Should register hooks.
	*	@param manager Manager.
	*	@param hookManager Hook manager.
	*	@return true on success, false otherwise.
	*/
	virtual bool AddHooks( CASManager& manager, CASHookManager& hookManager ) = 0;

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