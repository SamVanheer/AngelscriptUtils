#include <cassert>
#include <cstdarg>

#include "Angelscript/util/ASUtil.h"

#include "Angelscript/wrapper/CASContext.h"
#include "Angelscript/wrapper/ASCallable.h"

#include "ContextUtils.h"

namespace ctx
{
bool SetArguments( const asIScriptFunction& targetFunc, asIScriptContext& context, va_list list )
{
	assert( list );

	if( !list )
		return false;

	const asUINT uiArgCount = targetFunc.GetParamCount();

	bool bSuccess = true;

	asIScriptEngine& engine = *targetFunc.GetEngine();

	for( asUINT uiIndex = 0; uiIndex < uiArgCount && bSuccess; ++uiIndex )
	{
		bSuccess = SetContextArgument( engine, targetFunc, context, uiIndex, list );
	}

	return bSuccess;
}

bool SetContextArgument( asIScriptEngine& engine, const asIScriptFunction& targetFunc, asIScriptContext& context, asUINT uiIndex, va_list& list )
{
	int iTypeId;
	asDWORD uiFlags;

	if( targetFunc.GetParam( uiIndex, &iTypeId, &uiFlags ) < 0 )
	{
		//TODO
		//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::SetContextArgument: an error occurred while getting function parameter information, aborting!\n" );
		return false;
	}

	bool bSuccess = true;

	ArgumentValue_t value;

	if( ( bSuccess = GetArgumentFromVarargs( value, iTypeId, uiFlags, list ) ) )
	{
		//Remove the handle flag from the typeid.
		//Input should never be dereferenced
		bSuccess = SetContextArgument( engine, targetFunc, context, uiIndex, iTypeId & ~asTYPEID_OBJHANDLE, value, true );
	}

	return bSuccess;
}

bool GetArgumentFromVarargs( ArgumentValue_t& value, int iTypeId, asDWORD uiTMFlags, va_list& list, asDWORD* puiObjFlags, ArgumentType_t* pOutArgType )
{
	if( pOutArgType && !puiObjFlags )
		return false;

	bool bSuccess = true;

	if( pOutArgType )
		*pOutArgType = AT_NONE;

	//Its an object (ref or value type), or handle to ref type
	//Handles reference parameters automatically, source pointer must be correct type
	if( iTypeId & ( asTYPEID_OBJHANDLE | asTYPEID_MASK_OBJECT ) )
	{
		value.pValue = va_arg( list, void* );

		//Set type if requested
		if( pOutArgType )
		{
			if( ( *puiObjFlags ) & asOBJ_REF )
				*pOutArgType = AT_REF;
			else if( ( *puiObjFlags ) & asOBJ_VALUE )
				*pOutArgType = AT_VALUE;
			else
			{
				//TODO
				//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::GetArgumentFromVarargs: unknown object type, cannot convert!\n" );
				bSuccess = false;
			}
		}
	}
	else //Primitive type (including enum)
	{
		bool bWasPrimitive = true;

		//In/Out reference
		if( as::IsPrimitive( iTypeId ) && ( uiTMFlags & ( asTM_INREF | asTM_OUTREF ) ) )
		{
			value.pValue = va_arg( list, void* );
		}
		else
		{
			switch( iTypeId )
			{
			case asTYPEID_VOID:
				{
					//Impossible
					//TODO
					//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::SetContextArgument: the impossible happened: a void argument\n" );
					bSuccess = false;
					break;
				}

			case asTYPEID_BOOL:
			case asTYPEID_INT8:
			case asTYPEID_UINT8:	value.byte = static_cast<asBYTE>( va_arg( list, long ) ); break; //Promoted to int
			case asTYPEID_INT16:
			case asTYPEID_UINT16:	value.word = static_cast<asWORD>( va_arg( list, long ) ); break; //Promoted to int
			case asTYPEID_INT32:
			case asTYPEID_UINT32:	value.dword = static_cast<asDWORD>( va_arg( list, long ) ); break;
			case asTYPEID_INT64:
			case asTYPEID_UINT64:	value.qword = va_arg( list, long long ); break;

			case asTYPEID_FLOAT:	value.flValue = static_cast<float>( va_arg( list, double ) ); break; //Promoted to double
			case asTYPEID_DOUBLE:	value.dValue = va_arg( list, double ); break;

			default:

				bWasPrimitive = false;

				//It's an enum
				if( as::IsEnum( iTypeId ) )
				{
					if( pOutArgType )
						*pOutArgType = AT_ENUM;

					value.dword = va_arg( list, long );
				}
				else
				{
					//TODO
					//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::SetContextArgument: Attempted to set parameter of unknown type, aborting!\n" );
					bSuccess = false;
				}
				break;
			}
		}

		if( pOutArgType && bWasPrimitive )
			*pOutArgType = AT_PRIMITIVE;
	}

	return bSuccess;
}

bool SetContextArgument( asIScriptEngine& engine, const asIScriptFunction& targetFunc, asIScriptContext& context,
										asUINT uiIndex, int iSourceTypeId, ArgumentValue_t& value, bool bAllowPrimitiveReferences )
{
	int iTypeId;
	asDWORD uiFlags;

	if( targetFunc.GetParam( uiIndex, &iTypeId, &uiFlags ) < 0 )
	{
		//TODO
		//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::SetContextArgument: an error occurred while getting function parameter information, aborting!\n" );
		return false;
	}

	bool bSuccess = true;

	//It's an object handle (ref type)
	if( iTypeId & asTYPEID_OBJHANDLE )
	{
		void* pSourceObj = ( iSourceTypeId & asTYPEID_OBJHANDLE ) ? *reinterpret_cast<void**>( value.pValue ) : value.pValue;

		if( ( iSourceTypeId & asTYPEID_MASK_OBJECT ) )
		{
			asITypeInfo* pType = engine.GetTypeInfoById( iTypeId );

			if( pType )
			{
				bool bCanSet = true;

				//Types are functions
				if( pType->GetFlags() & asOBJ_FUNCDEF )
				{
					asIScriptFunction* pSourceFunc = reinterpret_cast<asIScriptFunction*>( pSourceObj );

					//Functions are incompatible, can't set
					if( !pSourceFunc->IsCompatibleWithTypeId( iTypeId ) )
					{
						//TODO
						/*
						gASLog()->Error( ASLOG_CRITICAL,
										 "CASCPPReflection::SetContextArgument: Could not set argument %u, argument function signatures are different, aborting!\n", uiIndex );
										 */
						bCanSet = false;
					}
				}
				else
				{
					void* pObject = nullptr;

					if( engine.RefCastObject( pSourceObj, engine.GetTypeInfoById( iSourceTypeId ), pType, &pObject ) >= 0 )
						engine.ReleaseScriptObject( pObject, pType );
					else
					{
						//TODO
						//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::SetContextArgument: source argument is incompatible with target, aborting!\n" );
						bCanSet = false;
					}
				}

				if( bCanSet )
					bSuccess = context.SetArgObject( uiIndex, pSourceObj ) >= 0;
				else
					bSuccess = false;
			}
			else
			{
				//TODO
				//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::SetContextArgument: Could not get object type for argument %u, aborting!\n", uiIndex );
				bSuccess = false;
			}
		}
		else
		{
			//TODO
			//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::SetContextArgument: source argument is incompatible with target, aborting!\n" );
			bSuccess = false;
		}
	}
	else if( iTypeId & asTYPEID_MASK_OBJECT ) //Object value (ref or value type)
	{
		bool bIsCompatible = false;

		if( iTypeId == iSourceTypeId )
			bIsCompatible = true;

		if( bIsCompatible )
			bSuccess = context.SetArgObject( uiIndex, value.pValue ) >= 0;
	}
	else //Primitive type (including enum)
	{
		//Primitive type taken by reference
		if( ( as::IsPrimitive( iTypeId ) ) && ( uiFlags & ( asTM_INREF | asTM_OUTREF ) ) )
		{
			void* pAddress = bAllowPrimitiveReferences ? value.pValue : &value.qword;
			bSuccess = context.SetArgAddress( uiIndex, pAddress ) >= 0;
		}
		else
		{
			bool bWasPrimitive = true;

			//Needs a little conversion magic
			asINT64 uiValue;
			double dValue;

			//Type was a primitive type or enum
			bSuccess = ConvertInputArgToLargest( iSourceTypeId, value, uiValue, dValue );

			if( bSuccess )
			{
				switch( iTypeId )
				{
				case asTYPEID_VOID:
					{
						//Impossible
						//TODO
						//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::SetContextArgument: the impossible happened: a void argument, aborting!\n" );
						bSuccess = false;
						break;
					}

				case asTYPEID_BOOL:
				case asTYPEID_INT8:
				case asTYPEID_UINT8:	bSuccess = context.SetArgByte( uiIndex, static_cast<asBYTE>( uiValue ) ) >= 0; break;
				case asTYPEID_INT16:
				case asTYPEID_UINT16:	bSuccess = context.SetArgWord( uiIndex, static_cast<asWORD>( uiValue ) ) >= 0; break;
				case asTYPEID_INT32:
				case asTYPEID_UINT32:	bSuccess = context.SetArgDWord( uiIndex, static_cast<asDWORD>( uiValue ) ) >= 0; break;
				case asTYPEID_INT64:
				case asTYPEID_UINT64:	bSuccess = context.SetArgQWord( uiIndex, uiValue ) >= 0; break;

				case asTYPEID_FLOAT:	bSuccess = context.SetArgFloat( uiIndex, static_cast<float>( dValue ) ) >= 0; break;
				case asTYPEID_DOUBLE:	bSuccess = context.SetArgDouble( uiIndex, dValue ) >= 0; break;

				default:

					bWasPrimitive = false;

					//It's an enum
					if( as::IsEnum( iTypeId & asTYPEID_MASK_SEQNBR ) )
					{
						if( uiFlags & ( asTM_INREF | asTM_OUTREF ) )
						{
							void* pAddress = bAllowPrimitiveReferences ? value.pValue : &value.qword;
							bSuccess = context.SetArgAddress( uiIndex, pAddress ) >= 0;
						}
						else
							bSuccess = context.SetArgDWord( uiIndex, value.dword ) >= 0;
					}
					else
					{
						//TODO
						//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::SetContextArgument: Attempted to set parameter of unknown type, aborting!\n" );
						bSuccess = false;
					}
					break;
				}

				if( bWasPrimitive )
				{
					if( iTypeId != iSourceTypeId )
					{
						//TODO
						/*
						gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::SetContextArgument: Attempted to set primitive value of type '%s' to value of type '%s', aborting!\n",
										 PrimitiveTypeIdToString( iTypeId ), PrimitiveTypeIdToString( iSourceTypeId ) );
										 */
						bSuccess = false;
					}
				}
			}
		}
	}

	return bSuccess;
}

bool ConvertInputArgToLargest( const CASArgument* const pArg, asINT64& uiValue, double& flValue )
{
	if( !pArg )
		return false;

	return ConvertInputArgToLargest( pArg->GetTypeId(), pArg->GetArgumentValue(), uiValue, flValue );
}

bool ConvertInputArgToLargest( int iTypeId, const ArgumentValue_t& value, asINT64& uiValue, double& flValue )
{
	bool bSuccess = true;

	int iType = asTYPEID_UINT64;

	switch( iTypeId )
	{
	case asTYPEID_VOID:
		{
			//Impossible
			//TODO
			//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::ConvertInputArgToLargest: the impossible happened: a void argument, aborting!\n" );
			bSuccess = false;
			break;
		}

	case asTYPEID_BOOL:		uiValue = value.byte; break;

	case asTYPEID_INT8:
	case asTYPEID_UINT8:	uiValue = value.byte; break; //Promoted to int
	case asTYPEID_INT16:
	case asTYPEID_UINT16:	uiValue = value.word; break; //Promoted to int
	case asTYPEID_INT32:
	case asTYPEID_UINT32:	uiValue = value.dword; break;
	case asTYPEID_INT64:
	case asTYPEID_UINT64:	uiValue = value.qword; break;

	case asTYPEID_FLOAT:	flValue = value.flValue; iType = asTYPEID_DOUBLE; break; //Promoted to double
	case asTYPEID_DOUBLE:	flValue = value.dValue; iType = asTYPEID_DOUBLE; break;

	default:

		//It's an enum
		if( as::IsEnum( iTypeId ) )
		{
			uiValue = value.dword;
		}
		else
		{
			//Type isn't a primitive type or enum
			bSuccess = false;
		}
		break;
	}

	if( bSuccess )
	{
		//Both values must contain the same value
		if( iType == asTYPEID_UINT64 )
			flValue = static_cast<double>( uiValue );
		else
			uiValue = static_cast<asINT64>( flValue );
	}

	return bSuccess;
}

bool SetPrimitiveArgument( void* pData, int iTypeId, ArgumentValue_t& value, bool& bOutWasPrimitive )
{
	if( !pData )
	{
		bOutWasPrimitive = false;
		return false;
	}

	bOutWasPrimitive = true;

	bool bSuccess = true;

	//We handle signed and unsigned the same way; they occupy the same amount of memory, and will be cast when the destination function receives them.
	switch( iTypeId )
	{
	case asTYPEID_VOID:
		{
			//What the hell?! how did somebody pass a void argument?
			bSuccess = false;
			break;
		}

	case asTYPEID_BOOL:		value.byte = *reinterpret_cast<bool*>( pData ); break;

	case asTYPEID_INT8:
	case asTYPEID_UINT8:	value.byte = *reinterpret_cast<asBYTE*>( pData ); break;

	case asTYPEID_INT16:
	case asTYPEID_UINT16:	value.word = *reinterpret_cast<asWORD*>( pData ); break;

	case asTYPEID_INT32:
	case asTYPEID_UINT32:	value.dword = *reinterpret_cast<asDWORD*>( pData ); break;

	case asTYPEID_INT64:
	case asTYPEID_UINT64:	value.qword = *reinterpret_cast<asQWORD*>( pData ); break;

	case asTYPEID_FLOAT:	value.flValue = *reinterpret_cast<float*>( pData ); break;
	case asTYPEID_DOUBLE:	value.dValue = *reinterpret_cast<double*>( pData ); break;

	default: //Not a primitive type.
		{
			bOutWasPrimitive = false;
			break;
		}
	}

	return bSuccess;
}

#if 0

bool GetReturnValue( asIScriptContext& context, const asIScriptFunction& func, CASArgument& retVal, asDWORD* uiOutFlags )
{
	asDWORD uiFlags;
	const int iTypeId = func.GetReturnTypeId( &uiFlags );

	if( uiOutFlags )
		*uiOutFlags = uiFlags;

	return GetReturnValue( context, iTypeId, uiFlags, retVal );
}

bool GetReturnValue( asIScriptContext& context, int iTypeId, asDWORD uiFlags, CASArgument& retVal )
{
	bool bSuccess = true;

	ArgumentType_t argType = AT_NONE;
	ArgumentValue_t value;

	//Object handle type
	if( iTypeId & asTYPEID_OBJHANDLE )
	{
		argType = AT_REF;
		value.pValue = context.GetReturnObject();
	}
	else if( iTypeId & asTYPEID_MASK_OBJECT ) //Object type (ref or value type)
	{
		asIScriptEngine* pEngine = context.GetEngine();
		asITypeInfo* pType = pEngine->GetTypeInfoById( iTypeId );

		if( pType )
		{
			const asDWORD uiObjFlags = pType->GetFlags();

			value.pValue = context.GetReturnObject();

			if( uiObjFlags & asOBJ_REF )
			{
				argType = AT_REF;
			}
			else if( uiObjFlags & asOBJ_VALUE )
			{
				argType = AT_VALUE;

				//If the object is returned by value, a copy needs to be made
				//This is handled by CASArgument in the Set method below.
			}
			else
			{
				//TODO
				//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::GetReturnValue: unknown object type, cannot convert!\n" );
				bSuccess = false;
			}
		}
		else
		{
			//TODO
			//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::GetReturnValue: failed to get object type!\n" );
			bSuccess = false;
		}
	}
	else if( iTypeId == asTYPEID_VOID )
	{
		argType = AT_VOID;
	}
	else //Primitive type (including enum)
	{
		bool bWasPrimitive = true;

		//It's a reference
		if( uiFlags & asTM_INOUTREF )
		{
			if( !SetPrimitiveArgument( context.GetReturnAddress(), iTypeId, value, bWasPrimitive ) )
			{
				//TODO
				//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::GetReturnValue: Something went wrong while trying to convert the return value to a usable type!\n" );
				bSuccess = false;
			}
		}
		else //It's returned by value
		{
			switch( iTypeId )
			{
			case asTYPEID_BOOL:
			case asTYPEID_INT8:
			case asTYPEID_UINT8:	value.byte = context.GetReturnByte(); break;

			case asTYPEID_INT16:
			case asTYPEID_UINT16:	value.word = context.GetReturnWord(); break;

			case asTYPEID_INT32:
			case asTYPEID_UINT32:	value.dword = context.GetReturnDWord(); break;

			case asTYPEID_INT64:
			case asTYPEID_UINT64:	value.qword = context.GetReturnQWord(); break;

			case asTYPEID_FLOAT:	value.flValue = context.GetReturnFloat(); break;
			case asTYPEID_DOUBLE:	value.dValue = context.GetReturnDouble(); break;

			default: //Not a primitive type.
				{
					bWasPrimitive = false;
					break;
				}
			}
		}

		if( bSuccess )
		{
			if( bWasPrimitive )
				argType = AT_PRIMITIVE;
			else
			{
				if( as::IsEnum( iTypeId & asTYPEID_MASK_SEQNBR ) )
				{
					argType = AT_ENUM;
					value.dword = context.GetReturnDWord();
				}
				else
				{
					//TODO
					//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::GetReturnValue: unknown primitive return type, cannot convert!\n" );
					bSuccess = false;
				}
			}
		}
	}

	//Let the argument class handle copying
	retVal.Set( iTypeId, argType, value, true );

	return bSuccess;
}
#endif

bool GetReturnValue( asIScriptContext& context, int iTypeId, asDWORD uiFlags, void* pReturnValue )
{
	assert( pReturnValue );

	if( !pReturnValue )
		return false;

	bool bSuccess = true;

	//Object handle type
	if( iTypeId & asTYPEID_OBJHANDLE )
	{
		*reinterpret_cast<void**>( pReturnValue ) = context.GetReturnObject();
	}
	else if( iTypeId & asTYPEID_MASK_OBJECT ) //Object type (ref or value type)
	{
		asIScriptEngine* pEngine = context.GetEngine();
		asITypeInfo* pType = pEngine->GetTypeInfoById( iTypeId );

		if( pType )
		{
			void* pObject = context.GetReturnObject();

			asIScriptFunction* pFunc;

			asUINT uiIndex;

			const asUINT uiCount = pType->GetMethodCount();

			//Find an assignment operator
			for( uiIndex = 0; uiIndex < uiCount; ++uiIndex )
			{
				pFunc = pType->GetMethodByIndex( uiIndex );

				if( pFunc->GetParamCount() == 1 )
				{
					int iFuncTypeId;
					asDWORD uiFuncFlags;

					if( pFunc->GetParam( 0, &iFuncTypeId, &uiFuncFlags ) >= 0 )
					{
						if( iFuncTypeId == pType->GetTypeId() && uiFuncFlags & asTM_INREF )
						{
							context.PushState();

							//Call the assignment operator on the object
							CASContext ctx( context );

							CASMethod func( *pFunc, ctx, pReturnValue );

							func( CallFlag::NONE, pObject );

							context.PopState();

							break;
						}
					}
				}
			}

			if( uiIndex == uiCount )
			{
				bSuccess = false;
				//TODO
				/*
				gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::GetReturnValue: no assignment operator found for type '%s::%s', cannot convert!\n",
								 pType->GetNamespace(), pType->GetName() );
								 */
			}
		}
		else
		{
			//TODO
			//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::GetReturnValue: failed to get object type!\n" );
			bSuccess = false;
		}
	}
	else if( iTypeId == asTYPEID_VOID )
	{
		//TODO
		//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::GetReturnValue: type is null!\n" );
	}
	else //Primitive type (including enum)
	{
		bool bWasPrimitive = true;

		//It's a reference
		if( uiFlags & asTM_INOUTREF )
		{
			*reinterpret_cast<void**>( pReturnValue ) = context.GetReturnAddress();
		}
		else //It's returned by value
		{
			switch( iTypeId )
			{
			case asTYPEID_BOOL:		*reinterpret_cast<bool*>( pReturnValue ) = context.GetReturnByte() != 0; break;
			case asTYPEID_INT8:
			case asTYPEID_UINT8:	*reinterpret_cast<asBYTE*>( pReturnValue ) = context.GetReturnByte(); break;

			case asTYPEID_INT16:
			case asTYPEID_UINT16:	*reinterpret_cast<asWORD*>( pReturnValue ) = context.GetReturnWord(); break;

			case asTYPEID_INT32:
			case asTYPEID_UINT32:	*reinterpret_cast<asDWORD*>( pReturnValue ) = context.GetReturnDWord(); break;

			case asTYPEID_INT64:
			case asTYPEID_UINT64:	*reinterpret_cast<asQWORD*>( pReturnValue ) = context.GetReturnQWord(); break;

			case asTYPEID_FLOAT:	*reinterpret_cast<float*>( pReturnValue ) = context.GetReturnFloat(); break;
			case asTYPEID_DOUBLE:	*reinterpret_cast<double*>( pReturnValue ) = context.GetReturnDouble(); break;

			default: //Not a primitive type.
				{
					bWasPrimitive = false;
					break;
				}
			}
		}

		if( bSuccess )
		{
			if( !bWasPrimitive )
			{
				if( as::IsEnum( iTypeId & asTYPEID_MASK_SEQNBR ) )
				{
					*reinterpret_cast<asDWORD*>( pReturnValue ) = context.GetReturnDWord();
				}
				else
				{
					//TODO
					//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::GetReturnValue: unknown primitive return type, cannot convert!\n" );
					bSuccess = false;
				}
			}
		}
	}

	return bSuccess;
}
}