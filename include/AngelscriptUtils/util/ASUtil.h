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
*	Iterates over a list of functions.
*/
struct CASFunctionIterator final
{
	const asIScriptModule& module;

	/**
	*	Constructor.
	*	@param module Script module.
	*/
	CASFunctionIterator( const asIScriptModule& module )
		: module( module )
	{
	}

	CASFunctionIterator( const CASFunctionIterator& other ) = default;

	/**
	*	@return Number of functions.
	*/
	asUINT GetCount() const { return module.GetFunctionCount(); }

	/**
	*	Gets a function by index.
	*	@param uiIndex Function index.
	*	@return Function.
	*/
	asIScriptFunction* GetByIndex( const asUINT uiIndex ) const
	{
		return module.GetFunctionByIndex( uiIndex );
	}

private:
	CASFunctionIterator& operator=( const CASFunctionIterator& ) = delete;
};

/**
*	Iterates over a list of object methods.
*/
struct CASMethodIterator final
{
	const asITypeInfo& objectType;

	/**
	*	Constructor.
	*	@param objectType Object type info.
	*/
	CASMethodIterator( const asITypeInfo& objectType )
		: objectType( objectType )
	{
		//Must be an object.
		//TODO: asOBJ_VALUE should also count - Solokiller
		assert( objectType.GetFlags() & ( asOBJ_REF ) );
	}

	CASMethodIterator( const CASMethodIterator& other ) = default;

	/**
	*	@return Number of methods.
	*/
	asUINT GetCount() const { return objectType.GetMethodCount(); }

	/**
	*	Gets a method by index.
	*	@param uiIndex Function index.
	*	@return Method.
	*/
	asIScriptFunction* GetByIndex( const asUINT uiIndex ) const
	{
		return objectType.GetMethodByIndex( uiIndex );
	}

private:
	CASMethodIterator& operator=( const CASMethodIterator& ) = delete;
};

/**
*	Finds a function by name and argument types.
*	@param engine Script engine.
*	@param funcIterator Function iterator.
*	@param szFunctionName Name of the function.
*	@param arguments Function arguments.
*	@param bExplicitReturnType Whether the return type should be checked. Compared against iReturnTypeId.
*	@param iReturnTypeId The return type id to match against if bExplicitReturnType is true.
*	@return Function, or null if no function could be found.
*/
template<typename FUNCITERATOR>
inline asIScriptFunction* FindFunction(
	asIScriptEngine& engine,
	const FUNCITERATOR& funcIterator,
	const std::string& szFunctionName,
	CASArguments& arguments,
	const bool bExplicitReturnType = true,
	const int iReturnTypeId = asTYPEID_VOID )
{
	const auto& argList = arguments.GetArgumentList();

	for( asUINT uiIndex = 0; uiIndex < funcIterator.GetCount(); ++uiIndex )
	{
		asIScriptFunction* const pFunction = funcIterator.GetByIndex( uiIndex );

		//Name must match
		if( strcmp( pFunction->GetName(), szFunctionName.c_str() ) != 0 )
			continue;

		if( bExplicitReturnType )
		{
			if( pFunction->GetReturnTypeId() != iReturnTypeId )
				continue;
		}

		//Must match parameter count
		if( pFunction->GetParamCount() != arguments.GetArgumentCount() )
			continue;

		asUINT uiParamIndex;

		const asUINT uiParamCount = pFunction->GetParamCount();

		//Each parameter must be the correct type
		for( uiParamIndex = 0; uiParamIndex < uiParamCount; ++uiParamIndex )
		{
			int iTypeId;
			asDWORD uiFlags;

			if( pFunction->GetParam( uiParamIndex, &iTypeId, &uiFlags ) < 0 )
			{
				const auto szCandidateFuncName = asutils::FormatFunctionName( *pFunction );
				as::log->critical( "as::FindFunction: Failed to retrieve parameter {} for function {}!", uiParamIndex, szCandidateFuncName );
				break;
			}

			//Can be null in case of primitive types or enums
			auto pType = engine.GetTypeInfoById( iTypeId );

			const auto& arg = argList[ uiParamIndex ];

			const ArgType::ArgType argType = ctx::ArgumentTypeFromTypeId( iTypeId, pType ? pType->GetFlags() : 0 );

			if( arg.GetArgumentType() != argType )
			{
				//Enum to primitive conversion is handled by the next check
				if( arg.GetArgumentType() != ArgType::ENUM || argType != ArgType::PRIMITIVE )
					break;
			}

			//Make sure only to consider the base id + object flag
			if( ( arg.GetTypeId() & ( asTYPEID_MASK_OBJECT | asTYPEID_MASK_SEQNBR ) ) != ( iTypeId & ( asTYPEID_MASK_OBJECT | asTYPEID_MASK_SEQNBR ) ) )
			{
				ArgumentValue value;

				//Warning: either of these can modify value
				if( ctx::ConvertEnumToPrimitive( arg, iTypeId, value ) )
				{
					//TODO: handle conversion during call.
					/*
					//Change the type to match
					if( !arg.Set( iTypeId, arg.GetArgumentType(), value, false ) )
					{
						as::log->critical( "as::FindFunction: Failed to convert enum value!" );
						break;
					}
					*/
				}
				else
					break;
			}
		}

		//Validation passed
		if( uiParamIndex == uiParamCount )
		{
			return pFunction;
		}
	}

	return nullptr;
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