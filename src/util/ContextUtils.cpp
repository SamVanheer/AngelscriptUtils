#include <cassert>
#include <cstdarg>

#include "util/ASUtil.h"

#include "wrapper/CASContext.h"
#include "wrapper/ASCallable.h"

#include "ContextUtils.h"

namespace ctx
{
bool SetArguments( asIScriptFunction& targetFunc, asIScriptContext& context, va_list list )
{
	assert( list );

	if( !list )
		return false;

	const asUINT uiArgCount = targetFunc.GetParamCount();

	bool fSuccess = true;

	asIScriptEngine& engine = *targetFunc.GetEngine();

	for( asUINT uiIndex = 0; uiIndex < uiArgCount && fSuccess; ++uiIndex )
	{
		fSuccess = SetContextArgument( engine, targetFunc, context, uiIndex, list );
	}

	return fSuccess;
}

bool SetContextArgument( asIScriptEngine& engine, asIScriptFunction& targetFunc, asIScriptContext& context, asUINT uiIndex, va_list& list )
{
	int iTypeId;
	asDWORD uiFlags;

	if( targetFunc.GetParam( uiIndex, &iTypeId, &uiFlags ) < 0 )
	{
		//TODO
		//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::SetContextArgument: an error occurred while getting function parameter information, aborting!\n" );
		return false;
	}

	bool fSuccess = true;

	ArgumentValue_t value;

	if( ( fSuccess = GetArgumentFromVarargs( value, iTypeId, uiFlags, list ) ) )
	{
		//Remove the handle flag from the typeid.
		//Input should never be dereferenced
		fSuccess = SetContextArgument( engine, targetFunc, context, uiIndex, iTypeId & ~asTYPEID_OBJHANDLE, value, true );
	}

	return fSuccess;
}

bool GetArgumentFromVarargs( ArgumentValue_t& value, int iTypeId, asDWORD uiTMFlags, va_list& list, asDWORD* puiObjFlags, ArgumentType_t* pOutArgType )
{
	if( pOutArgType && !puiObjFlags )
		return false;

	bool fSuccess = true;

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
				fSuccess = false;
			}
		}
	}
	else //Primitive type (including enum)
	{
		bool fWasPrimitive = true;

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
					fSuccess = false;
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

				fWasPrimitive = false;

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
					fSuccess = false;
				}
				break;
			}
		}

		if( pOutArgType && fWasPrimitive )
			*pOutArgType = AT_PRIMITIVE;
	}

	return fSuccess;
}

bool SetContextArgument( asIScriptEngine& engine, asIScriptFunction& targetFunc, asIScriptContext& context,
										asUINT uiIndex, int iSourceTypeId, ArgumentValue_t& value, bool fAllowPrimitiveReferences )
{
	int iTypeId;
	asDWORD uiFlags;

	if( targetFunc.GetParam( uiIndex, &iTypeId, &uiFlags ) < 0 )
	{
		//TODO
		//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::SetContextArgument: an error occurred while getting function parameter information, aborting!\n" );
		return false;
	}

	bool fSuccess = true;

	//It's an object handle (ref type)
	if( iTypeId & asTYPEID_OBJHANDLE )
	{
		void* pSourceObj = ( iSourceTypeId & asTYPEID_OBJHANDLE ) ? *reinterpret_cast<void**>( value.pValue ) : value.pValue;

		if( ( iSourceTypeId & asTYPEID_MASK_OBJECT ) )
		{
			asITypeInfo* pType = engine.GetTypeInfoById( iTypeId );

			if( pType )
			{
				bool fCanSet = true;

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
						fCanSet = false;
					}
				}
				else
				{
					void* pObject = NULL;

					if( engine.RefCastObject( pSourceObj, engine.GetTypeInfoById( iSourceTypeId ), pType, &pObject ) >= 0 )
						engine.ReleaseScriptObject( pObject, pType );
					else
					{
						//TODO
						//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::SetContextArgument: source argument is incompatible with target, aborting!\n" );
						fCanSet = false;
					}
				}

				if( fCanSet )
					fSuccess = context.SetArgObject( uiIndex, pSourceObj ) >= 0;
				else
					fSuccess = false;
			}
			else
			{
				//TODO
				//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::SetContextArgument: Could not get object type for argument %u, aborting!\n", uiIndex );
				fSuccess = false;
			}
		}
		else
		{
			//TODO
			//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::SetContextArgument: source argument is incompatible with target, aborting!\n" );
			fSuccess = false;
		}
	}
	else if( iTypeId & asTYPEID_MASK_OBJECT ) //Object value (ref or value type)
	{
		bool fIsCompatible = false;

		if( iTypeId == iSourceTypeId )
			fIsCompatible = true;

		if( fIsCompatible )
			fSuccess = context.SetArgObject( uiIndex, value.pValue ) >= 0;
	}
	else //Primitive type (including enum)
	{
		//Primitive type taken by reference
		if( ( as::IsPrimitive( iTypeId ) ) && ( uiFlags & ( asTM_INREF | asTM_OUTREF ) ) )
		{
			void* pAddress = fAllowPrimitiveReferences ? value.pValue : &value.qword;
			fSuccess = context.SetArgAddress( uiIndex, pAddress ) >= 0;
		}
		else
		{
			bool fWasPrimitive = true;

			//Needs a little conversion magic
			asINT64 uiValue;
			double dValue;

			//Type was a primitive type or enum
			fSuccess = ConvertInputArgToLargest( iSourceTypeId, value, uiValue, dValue );

			if( fSuccess )
			{
				switch( iTypeId )
				{
				case asTYPEID_VOID:
					{
						//Impossible
						//TODO
						//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::SetContextArgument: the impossible happened: a void argument, aborting!\n" );
						fSuccess = false;
						break;
					}

				case asTYPEID_BOOL:
				case asTYPEID_INT8:
				case asTYPEID_UINT8:	fSuccess = context.SetArgByte( uiIndex, static_cast<asBYTE>( uiValue ) ) >= 0; break;
				case asTYPEID_INT16:
				case asTYPEID_UINT16:	fSuccess = context.SetArgWord( uiIndex, static_cast<asWORD>( uiValue ) ) >= 0; break;
				case asTYPEID_INT32:
				case asTYPEID_UINT32:	fSuccess = context.SetArgDWord( uiIndex, static_cast<asDWORD>( uiValue ) ) >= 0; break;
				case asTYPEID_INT64:
				case asTYPEID_UINT64:	fSuccess = context.SetArgQWord( uiIndex, uiValue ) >= 0; break;

				case asTYPEID_FLOAT:	fSuccess = context.SetArgFloat( uiIndex, static_cast<float>( dValue ) ) >= 0; break;
				case asTYPEID_DOUBLE:	fSuccess = context.SetArgDouble( uiIndex, dValue ) >= 0; break;

				default:

					fWasPrimitive = false;

					//It's an enum
					if( as::IsEnum( iTypeId & asTYPEID_MASK_SEQNBR ) )
					{
						if( uiFlags & ( asTM_INREF | asTM_OUTREF ) )
						{
							void* pAddress = fAllowPrimitiveReferences ? value.pValue : &value.qword;
							fSuccess = context.SetArgAddress( uiIndex, pAddress ) >= 0;
						}
						else
							fSuccess = context.SetArgDWord( uiIndex, value.dword ) >= 0;
					}
					else
					{
						//TODO
						//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::SetContextArgument: Attempted to set parameter of unknown type, aborting!\n" );
						fSuccess = false;
					}
					break;
				}

				if( fWasPrimitive )
				{
					if( iTypeId != iSourceTypeId )
					{
						//TODO
						/*
						gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::SetContextArgument: Attempted to set primitive value of type '%s' to value of type '%s', aborting!\n",
										 PrimitiveTypeIdToString( iTypeId ), PrimitiveTypeIdToString( iSourceTypeId ) );
										 */
						fSuccess = false;
					}
				}
			}
		}
	}

	return fSuccess;
}

bool ConvertInputArgToLargest( const CASArgument* const pArg, asINT64& uiValue, double& dValue )
{
	if( !pArg )
		return false;

	return ConvertInputArgToLargest( pArg->GetTypeId(), pArg->GetArgumentValue(), uiValue, dValue );
}

bool ConvertInputArgToLargest( int iTypeId, const ArgumentValue_t& value, asINT64& uiValue, double& dValue )
{
	bool fSuccess = true;

	int iType = asTYPEID_UINT64;

	switch( iTypeId )
	{
	case asTYPEID_VOID:
		{
			//Impossible
			//TODO
			//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::ConvertInputArgToLargest: the impossible happened: a void argument, aborting!\n" );
			fSuccess = false;
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

	case asTYPEID_FLOAT:	dValue = value.flValue; iType = asTYPEID_DOUBLE; break; //Promoted to double
	case asTYPEID_DOUBLE:	dValue = value.dValue; iType = asTYPEID_DOUBLE; break;

	default:

		//It's an enum
		if( as::IsEnum( iTypeId ) )
		{
			uiValue = value.dword;
		}
		else //Type isn't a primitive type or enum
			fSuccess = false;
		break;
	}

	if( fSuccess )
	{
		//Both values must contain the same value
		if( iType == asTYPEID_UINT64 )
			dValue = static_cast<double>( uiValue );
		else
			uiValue = static_cast<asINT64>( dValue );
	}

	return fSuccess;
}

bool SetPrimitiveArgument( ArgumentValue_t& value, int iTypeId, void* pData, bool& fOutWasPrimitive )
{
	if( !pData )
	{
		fOutWasPrimitive = false;
		return false;
	}

	fOutWasPrimitive = true;

	bool fSuccess = true;

	//We handle signed and unsigned the same way, they occupy the same amount of memory, and will be cast when the destination function receives them.
	switch( iTypeId )
	{
	case asTYPEID_VOID:
		{
			//What the hell?! how did somebody pass a void argument?
			fSuccess = false;
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
			fOutWasPrimitive = false;
			break;
		}
	}

	return fSuccess;
}

#if 0

bool GetReturnValue( CASArgument& retVal, asIScriptFunction* pFunc, asIScriptContext* pContext, asDWORD* uiOutFlags )
{
	if( !pFunc || !pContext )
		return false;

	asDWORD uiFlags;
	int iTypeId = pFunc->GetReturnTypeId( &uiFlags );

	if( uiOutFlags )
		*uiOutFlags = uiFlags;

	return GetReturnValue( retVal, iTypeId, uiFlags, pContext );
}

bool GetReturnValue( CASArgument& retVal, int iTypeId, asDWORD uiFlags, asIScriptContext* pContext )
{
	if( !pContext )
		return false;

	bool fSuccess = true;

	ArgumentType_t argType = AT_NONE;
	ArgumentValue_t value;

	//Object handle type
	if( iTypeId & asTYPEID_OBJHANDLE )
	{
		argType = AT_REF;
		value.pValue = pContext->GetReturnObject();
	}
	else if( iTypeId & asTYPEID_MASK_OBJECT ) //Object type (ref or value type)
	{
		asIScriptEngine* pEngine = pContext->GetEngine();
		asITypeInfo* pType = pEngine->GetTypeInfoById( iTypeId );

		if( pType )
		{
			const asDWORD uiObjFlags = pType->GetFlags();

			value.pValue = pContext->GetReturnObject();

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
				fSuccess = false;
			}
		}
		else
		{
			//TODO
			//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::GetReturnValue: failed to get object type!\n" );
			fSuccess = false;
		}
	}
	else if( iTypeId == asTYPEID_VOID )
	{
		argType = AT_VOID;
	}
	else //Primitive type (including enum)
	{
		bool fWasPrimitive = true;

		//It's a reference
		if( uiFlags & asTM_INOUTREF )
		{
			if( !SetPrimitiveArgument( value, iTypeId, pContext->GetReturnAddress(), fWasPrimitive ) )
			{
				//TODO
				//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::GetReturnValue: Something went wrong while trying to convert the return value to a usable type!\n" );
				fSuccess = false;
			}
		}
		else //It's returned by value
		{
			switch( iTypeId )
			{
			case asTYPEID_BOOL:
			case asTYPEID_INT8:
			case asTYPEID_UINT8:	value.byte = pContext->GetReturnByte(); break;

			case asTYPEID_INT16:
			case asTYPEID_UINT16:	value.word = pContext->GetReturnWord(); break;

			case asTYPEID_INT32:
			case asTYPEID_UINT32:	value.dword = pContext->GetReturnDWord(); break;

			case asTYPEID_INT64:
			case asTYPEID_UINT64:	value.qword = pContext->GetReturnQWord(); break;

			case asTYPEID_FLOAT:	value.flValue = pContext->GetReturnFloat(); break;
			case asTYPEID_DOUBLE:	value.dValue = pContext->GetReturnDouble(); break;

			default: //Not a primitive type.
				{
					fWasPrimitive = false;
					break;
				}
			}
		}

		if( fSuccess )
		{
			if( fWasPrimitive )
				argType = AT_PRIMITIVE;
			else
			{
				if( as::IsEnum( iTypeId & asTYPEID_MASK_SEQNBR ) )
				{
					argType = AT_ENUM;
					value.dword = pContext->GetReturnDWord();
				}
				else
				{
					//TODO
					//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::GetReturnValue: unknown primitive return type, cannot convert!\n" );
					fSuccess = false;
				}
			}
		}
	}

	//Let the argument class handle copying
	retVal.Set( iTypeId, argType, value, true );

	return fSuccess;
}
#endif

bool GetReturnValue( void* pReturnValue, int iTypeId, asDWORD uiFlags, asIScriptContext* pContext )
{
	if( !pContext )
		return false;

	bool fSuccess = true;

	//Object handle type
	if( iTypeId & asTYPEID_OBJHANDLE )
	{
		*reinterpret_cast<void**>( pReturnValue ) = pContext->GetReturnObject();
	}
	else if( iTypeId & asTYPEID_MASK_OBJECT ) //Object type (ref or value type)
	{
		asIScriptEngine* pEngine = pContext->GetEngine();
		asITypeInfo* pType = pEngine->GetTypeInfoById( iTypeId );

		if( pType )
		{
			void* pObject = pContext->GetReturnObject();

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
							pContext->PushState();

							//Call the assignment operator on the object
							CASContext ctx( *pContext );

							CASMethod func( *pFunc, ctx, pReturnValue );

							func( CallFlag::NONE, pObject );

							pContext->PopState();

							break;
						}
					}
				}
			}

			if( uiIndex == uiCount )
			{
				fSuccess = false;
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
			fSuccess = false;
		}
	}
	else if( iTypeId == asTYPEID_VOID )
	{
		//TODO
		//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::GetReturnValue: type is null!\n" );
	}
	else //Primitive type (including enum)
	{
		bool fWasPrimitive = true;

		//It's a reference
		if( uiFlags & asTM_INOUTREF )
		{
			*reinterpret_cast<void**>( pReturnValue ) = pContext->GetReturnAddress();
		}
		else //It's returned by value
		{
			switch( iTypeId )
			{
			case asTYPEID_BOOL:		*reinterpret_cast<bool*>( pReturnValue ) = pContext->GetReturnByte() != 0; break;
			case asTYPEID_INT8:
			case asTYPEID_UINT8:	*reinterpret_cast<asBYTE*>( pReturnValue ) = pContext->GetReturnByte(); break;

			case asTYPEID_INT16:
			case asTYPEID_UINT16:	*reinterpret_cast<asWORD*>( pReturnValue ) = pContext->GetReturnWord(); break;

			case asTYPEID_INT32:
			case asTYPEID_UINT32:	*reinterpret_cast<asDWORD*>( pReturnValue ) = pContext->GetReturnDWord(); break;

			case asTYPEID_INT64:
			case asTYPEID_UINT64:	*reinterpret_cast<asQWORD*>( pReturnValue ) = pContext->GetReturnQWord(); break;

			case asTYPEID_FLOAT:	*reinterpret_cast<float*>( pReturnValue ) = pContext->GetReturnFloat(); break;
			case asTYPEID_DOUBLE:	*reinterpret_cast<double*>( pReturnValue ) = pContext->GetReturnDouble(); break;

			default: //Not a primitive type.
				{
					fWasPrimitive = false;
					break;
				}
			}
		}

		if( fSuccess )
		{
			if( !fWasPrimitive )
			{
				if( as::IsEnum( iTypeId & asTYPEID_MASK_SEQNBR ) )
				{
					*reinterpret_cast<asDWORD*>( pReturnValue ) = pContext->GetReturnDWord();
				}
				else
				{
					//TODO
					//gASLog()->Error( ASLOG_CRITICAL, "CASCPPReflection::GetReturnValue: unknown primitive return type, cannot convert!\n" );
					fSuccess = false;
				}
			}
		}
	}

	return fSuccess;
}
}