#ifndef ANGELSCRIPT_UTIL_ASEXTENDADAPTER_H
#define ANGELSCRIPT_UTIL_ASEXTENDADAPTER_H

#include <string>

class IASExtendAdapter;

/**
*	@defgroup ASExtend Angelscript Class Extension Framework
*
*	@{
*/

namespace as
{
/**
*	Creates a class declaration for an extension class that scripts can inherit from.
*	@param pszClassName The class name that the class will have.
*	@param pszBaseClassName Class or interface to inherit from. Can be a list of classes/interfaces, separated by commas.
*	@param pszCPPClassName Registered name of the class that represents the C++ version of the class. Will be represented by the self member.
*	@param pszCPPBaseClassName Registered name of the class that provides base class method calling features. 
*								Will be represented by the BaseClass member.
*								Can be null, in which case no BaseClass member is defined.
*	@param pszClassContents Optional. Contents to add to the class in addition to those provided by the framework.
*	@return String containing the class declaration.
*/
std::string CreateExtendBaseclassDeclaration(
	const char* const pszClassName, const char* const pszBaseClassName,
	const char* const pszCPPClassName, const char* const pszCPPBaseClassName, 
	const char* const pszClassContents = nullptr );

/**
*	Initializes the extension class.
*	@param adapter The adapter representing the class.
*	@param pThis This pointer to set. Usually the same as &adapter, but could be different when using multiple inheritance.
*	@param pszCPPClassName Registered name of the class that represents the C++ version of the class.
*	@param pszCPPBaseClassName Registered name of the class that provides base class method calling features. Can be null, in which case no BaseClass member is initialized.
*	@return true on success, false otherwise.
*/
bool InitializeExtendClass( 
	IASExtendAdapter& adapter, void* const pThis, 
	const char* const pszCPPClassName, const char* const pszCPPBaseClassName );
}

/** @} */

#endif //ANGELSCRIPT_UTIL_ASEXTENDADAPTER_H