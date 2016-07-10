#ifndef UTIL_CONTEXTUTILS_H
#define UTIL_CONTEXTUTILS_H

#include <angelscript.h>

#include "wrapper/CASArguments.h"

namespace ctx
{
/**
*	Sets arguments for a function call.
*/
bool SetArguments( asIScriptFunction& targetFunc, asIScriptContext& context, va_list list );

/**
*	Sets an argument on the context, taking the argument from varargs.
*/
bool SetContextArgument( asIScriptEngine& engine, asIScriptFunction& targetFunc, asIScriptContext& context, asUINT uiIndex, va_list& list );

/**
*	Gets the argument of type iTypeId from the stack, and stores it in value
*	uiTMFlags is a combination of asETypeModifiers, or asTM_NONE
*	If pOutArgType is set, will also retrieve object type and store it there
*	puiObjFlags must be set if pOutArgType is set
*/
bool GetArgumentFromVarargs( ArgumentValue_t& value, int iTypeId, asDWORD uiTMFlags, va_list& list, asDWORD* puiObjFlags = nullptr, ArgumentType_t* pOutArgType = nullptr );

/**
*	Sets an argument on the context, taking the argument from an ArgumentValue_t
*	fAllowPrimitiveReferences indicates whether primitive type arguments taken by reference use pValue or &qword
*/
bool SetContextArgument( asIScriptEngine& engine, asIScriptFunction& targetFunc, asIScriptContext& context,
						 asUINT uiIndex, int iSourceTypeId, ArgumentValue_t& value, bool fAllowPrimitiveReferences );

/**
*	Converts the input argument to either an asQWORD or double.
*	iType is either asTYPEID_UINT64 or asTYPEID_DOUBLE
*	Returns false if conversion failed.
*/
bool ConvertInputArgToLargest( const CASArgument* const pArg, asINT64& uiValue, double& dValue );

/**
*	@see ConvertInputArgToLargest( const CASArgument* const pArg, asINT64& uiValue, double& dValue )
*/
bool ConvertInputArgToLargest( int iTypeId, const ArgumentValue_t& value, asINT64& uiValue, double& dValue );
}

#endif //UTIL_CONTEXTUTILS_H