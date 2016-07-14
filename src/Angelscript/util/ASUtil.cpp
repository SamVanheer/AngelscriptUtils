#include <cstdint>
#include <memory>

#include "Angelscript/add_on/scriptany.h"

#include "ASUtil.h"

namespace as
{
const char* PrimitiveTypeIdToString( int iTypeId )
{
	switch( iTypeId )
	{
	case asTYPEID_VOID:		return "void";
	case asTYPEID_BOOL:		return "bool";
	case asTYPEID_INT8:		return "int8";
	case asTYPEID_INT16:	return "int16";
	case asTYPEID_INT32:	return "int32";
	case asTYPEID_INT64:	return "int64";
	case asTYPEID_UINT8:	return "uint8";
	case asTYPEID_UINT16:	return "uint16";
	case asTYPEID_UINT32:	return "uint32";
	case asTYPEID_UINT64:	return "uint64";
	case asTYPEID_FLOAT:	return "float";
	case asTYPEID_DOUBLE:	return "double";
	default:				return nullptr;
	}
}

void ReleaseVarArg( asIScriptEngine& engine, void* pObject, const int iTypeId )
{
	if( !pObject )
		return;

	auto pTypeInfo = engine.GetTypeInfoById( iTypeId );

	//Handles are pointers to pointers, so dereference it.
	if( iTypeId & asTYPEID_OBJHANDLE )
	{
		pObject = *reinterpret_cast<void**>( pObject );
	}

	if( iTypeId & asTYPEID_MASK_OBJECT )
	{
		engine.ReleaseScriptObject( pObject, pTypeInfo );
	}
}

bool HasDefaultConstructor( const asITypeInfo& type )
{
	//Non-object types return 0 factories here.
	for( asUINT uiIndex = 0; uiIndex < type.GetFactoryCount(); ++uiIndex )
	{
		asIScriptFunction* pFactory = type.GetFactoryByIndex( uiIndex );

		//A default constructor has 0 parameters
		if( pFactory->GetParamCount() == 0 )
			return true;
	}

	return false;
}

void* CreateObjectInstance( asIScriptEngine& engine, const asITypeInfo& type )
{
	if( !HasDefaultConstructor( type ) )
		return nullptr;

	return engine.CreateScriptObject( &type );
}

CScriptAny* CreateScriptAny( asIScriptEngine& engine, void* pObject, int iTypeId )
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

	return new CScriptAny( pObject, iTypeId, &engine );
}

bool PODToString( char* pszBuffer, const size_t uiBufferSize, const void* pObject, const int iTypeId )
{
	assert( pszBuffer );
	assert( uiBufferSize > 0 );
	assert( pObject );

	int iResult;

	switch( iTypeId )
	{
	case asTYPEID_VOID:
		{
			//Treat as null handle
			iResult = snprintf( pszBuffer, uiBufferSize, "%p", nullptr );
			break;
		}

	case asTYPEID_BOOL:
		{
			iResult = snprintf( pszBuffer, uiBufferSize, *reinterpret_cast<const bool*>( pObject ) != 0 ? "true" : "false" );
			break;
		}

	case asTYPEID_INT8:
		{
			iResult = snprintf( pszBuffer, uiBufferSize, "%d", *reinterpret_cast<const char*>( pObject ) );
			break;
		}

	case asTYPEID_INT16:
		{
			iResult = snprintf( pszBuffer, uiBufferSize, "%d", *reinterpret_cast<const short*>( pObject ) );
			break;
		}

	case asTYPEID_INT32:
		{
			iResult = snprintf( pszBuffer, uiBufferSize, "%d", *reinterpret_cast<const long*>( pObject ) );
			break;
		}

	case asTYPEID_INT64:
		{
			iResult = snprintf( pszBuffer, uiBufferSize, "%lld", *reinterpret_cast<const long long*>( pObject ) );
			break;
		}

	case asTYPEID_UINT8:
		{
			iResult = snprintf( pszBuffer, uiBufferSize, "%u", *reinterpret_cast<const unsigned char*>( pObject ) );
			break;
		}

	case asTYPEID_UINT16:
		{
			iResult = snprintf( pszBuffer, uiBufferSize, "%u", *reinterpret_cast<const unsigned short*>( pObject ) );
			break;
		}

	case asTYPEID_UINT32:
		{
			iResult = snprintf( pszBuffer, uiBufferSize, "%u", *reinterpret_cast<const unsigned long*>( pObject ) );
			break;
		}

	case asTYPEID_UINT64:
		{
			iResult = snprintf( pszBuffer, uiBufferSize, "%llu", *reinterpret_cast<const unsigned long long*>( pObject ) );
			break;
		}

	case asTYPEID_FLOAT:
		{
			iResult = snprintf( pszBuffer, uiBufferSize, "%f", *reinterpret_cast<const float*>( pObject ) );
			break;
		}

	case asTYPEID_DOUBLE:
		{
			iResult = snprintf( pszBuffer, uiBufferSize, "%f", *reinterpret_cast<const double*>( pObject ) );
			break;
		}

	default:
		return false;
	}

	return iResult >= 0 && static_cast<size_t>( iResult ) < uiBufferSize;
}

bool SPrintf( char* pszBuffer, const size_t uiBufferSize, const char* pszFormat, size_t uiFirstParamIndex, asIScriptGeneric& arguments )
{
	if( !pszBuffer || !uiBufferSize || !pszFormat || uiFirstParamIndex > static_cast<size_t>( arguments.GetArgCount() ) )
		return false;

	//Set to empty result
	memset( pszBuffer, 0, uiBufferSize );

	//Nothing to print
	if( !( *pszFormat ) )
		return true;

	char* pszBufferDest = pszBuffer;

	const char* const pszBufferEnd = pszBuffer + uiBufferSize;

	//Total number of arguments - offset
	const size_t uiArgCount = static_cast<size_t>( arguments.GetArgCount() ) - uiFirstParamIndex;

	char szValueBuffer[ 1024 ];

	bool bSuccess = true;

	auto pEngine = arguments.GetEngine();

	//TODO: this should be rewritten to allow for format specifiers.
	while( *pszFormat )
	{
		if( *pszFormat == '%' )
		{
			//We've encountered a format parameter
			++pszFormat;

			if( *pszFormat == '%' )
			{
				//Just insert a %
				*pszBufferDest = '%';
				++pszBufferDest;
			}
			else
			{
				//Parse index
				char* pszEnd;
				const unsigned int uiIndex = strtoul( pszFormat, &pszEnd, 10 );

				const size_t uiIndexLength = pszEnd - pszFormat;

				//If the index is invalid, stop.
				if( uiIndex == 0 || uiIndex > uiArgCount )
				{
					//TODO
					//gASLog()->Warning( ASLOG_CRITICAL, "ASSPrintf: format parameter index is out of range!\n" );
					bSuccess = false;
					break;
				}
				else
				{
					//Index is 1 based: make 0 based and offset to first actual varargs parameter
					const asUINT uiArgIndex = ( uiIndex - 1 ) + uiFirstParamIndex;
					const int iTypeId = arguments.GetArgTypeId( uiArgIndex );

					szValueBuffer[ 0 ] = '\0';

					void* pValue = arguments.GetArgAddress( uiArgIndex );

					if( as::IsPrimitive( iTypeId ) )
					{
						bSuccess = PODToString( szValueBuffer, sizeof( szValueBuffer ), pValue, iTypeId );
					}
					else
					{
						auto pType = pEngine->GetTypeInfoById( iTypeId );

						if( pType )
						{
							//It's a string
							if( strcmp( pType->GetName(), "string" ) == 0 )
							{
								const auto& szString = *reinterpret_cast<const std::string*>( pValue );

								const size_t uiValueLength = szString.length();

								//Check to make sure it can fit, stop otherwise.
								if( uiValueLength >= static_cast<size_t>( pszBufferEnd - pszBufferDest ) )
								{
									//TODO
									//gASLog()->Error( ASLOG_CRITICAL, "as::SPrintf: could not fit data in buffer\n" );
									bSuccess = false;
									break;
								}

								strncat( pszBufferDest, szString.c_str(), ( pszBufferEnd - pszBufferDest ) - 1 );

								pszBufferDest += uiValueLength;
							}
							else
							{
								//Pointer types
								snprintf( szValueBuffer, sizeof( szValueBuffer ), "%p", pValue );
							}
						}
						else
						{
							if( pValue ) //Treat as dword
								snprintf( szValueBuffer, sizeof( szValueBuffer ), "%d", *reinterpret_cast<long*>( pValue ) );
							else //Treat as pointer
								snprintf( szValueBuffer, sizeof( szValueBuffer ), "%p", pValue );
						}
					}

					if( !bSuccess )
						break;

					const size_t uiValueLength = strlen( szValueBuffer );

					//Check to make sure it can fit, stop otherwise.
					if( uiValueLength >= static_cast<size_t>( pszBufferEnd - pszBufferDest ) )
					{
						//TODO
						//gASLog()->Error( ASLOG_CRITICAL, "as::SPrintf: could not fit data in buffer\n" );
						bSuccess = false;
						break;
					}

					strncat( pszBufferDest, szValueBuffer, ( pszBufferEnd - pszBufferDest ) - 1 );

					pszBufferDest += uiValueLength;

					pszFormat += uiIndexLength - 1; //Account for the ++ below
				}
			}
		}
		else
		{
			*pszBufferDest = *pszFormat;

			++pszBufferDest;

			if( pszBufferDest == pszBufferEnd )
			{
				//TODO
				//gASLog()->Error( ASLOG_CRITICAL, "as::SPrintf: could not fit data in buffer\n" );
				bSuccess = false;
				break;
			}
		}

		++pszFormat;
	}

	if( pszBufferDest == pszBufferEnd )
		return false;

	if( bSuccess )
		*pszBufferDest = '\0';

	return bSuccess;
}
}