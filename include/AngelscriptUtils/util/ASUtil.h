#ifndef UTIL_ASUTIL_H
#define UTIL_ASUTIL_H

#include <cassert>
#include <cctype>
#include <cstring>
#include <sstream>
#include <string>
#include <utility>

#include <angelscript.h>

#include "AngelscriptUtils/wrapper/CASArguments.h"

#include "AngelscriptUtils/util/ContextUtils.h"
#include "AngelscriptUtils/utility/TypeStringUtils.h"

#include "ASLogging.h"

#include "ASPlatform.h"

/**
*	@defgroup ASUtil Angelscript Utility Code
*
*	@{
*/

namespace as
{
/**
*	Releases a vararg argument.
*	@param engine Script engine.
*	@param pObject Object pointer.
*	@param iTypeId Type Id.
*/
void ReleaseVarArg( asIScriptEngine& engine, void* pObject, const int iTypeId );

/**
*	Creates an instance of an object using its default constructor.
*	@param engine Script engine.
*	@param type Object type.
*	@return Object instance, or null if the object could not be instantiated.
*/
void* CreateObjectInstance( asIScriptEngine& engine, const asITypeInfo& type );

/**
*	Sets a value on an any object, which is an instance of a class that has the interface defined by CScriptAny
*	Angelscript will select the int64 or double overload when any integer or float type is passed, so this is necessary.
*	@param any Script engine.
*	@param pObject Object to set in the any
*	@param iTypeId Type Id
*/
template<typename ANY>
void SetAny( ANY& any, void* pObject, int iTypeId )
{
	//Have to convert primitive types of the form signed int and float smaller than their largest so that any.retrieve works correctly.
	asINT64 iDest = 0;
	double flDest = 0.0;

	//TODO: do unsigned types work properly with any?

	switch( iTypeId )
	{
	case asTYPEID_INT8:
	case asTYPEID_UINT8:	iDest = *static_cast<int8_t*>( pObject ); break;
	case asTYPEID_INT16:
	case asTYPEID_UINT16:	iDest = *static_cast<int16_t*>( pObject ); break;
	case asTYPEID_INT32:
	case asTYPEID_UINT32:	iDest = *static_cast<int32_t*>( pObject ); break;

	case asTYPEID_FLOAT:	flDest = *static_cast<float*>( pObject ); break;

	default: break;
	}

	switch( iTypeId )
	{
	case asTYPEID_INT8:
	case asTYPEID_UINT8:
	case asTYPEID_INT16:
	case asTYPEID_UINT16:
	case asTYPEID_INT32:
	case asTYPEID_UINT32:
		pObject = &iDest;
		iTypeId = asTYPEID_INT64;
		break;

	case asTYPEID_FLOAT:
		pObject = &flDest;
		iTypeId = asTYPEID_DOUBLE;
		break;

	default: break;
	}

	any.Store( pObject, iTypeId );
}

/**
*	Sets a global variable by name.
*	@param module Module that contains the global variable.
*	@param pszGlobalName Name of the global variable to set.
*	@param value Value to set.
*	@return true on success, false otherwise.
*/
template<typename T>
inline bool SetGlobalByName( asIScriptModule& module, const char* const pszGlobalName, T value )
{
	int iIndex = module.GetGlobalVarIndexByName( pszGlobalName );

	if( iIndex >= 0 )
	{
		void* pGlobal = module.GetAddressOfGlobalVar( iIndex );

		*( reinterpret_cast<T*>( pGlobal ) ) = value;

		return true;
	}

	return false;
}

/**
*	Sets a global variable by name.
*	@param module Module that contains the global variable.
*	@param pszDecl Declaration of the global variable to set.
*	@param value Value to set.
*	@return true on success, false otherwise.
*/
template<typename T>
inline bool SetGlobalByDecl( asIScriptModule& module, const char* const pszDecl, T value )
{
	int iIndex = module.GetGlobalVarIndexByDecl( pszDecl );

	if( iIndex >= 0 )
	{
		void* pGlobal = module.GetAddressOfGlobalVar( iIndex );

		*( reinterpret_cast<T*>( pGlobal ) ) = value;

		return true;
	}

	return false;
}
}

/** @} */

#endif //UTIL_ASUTIL_H