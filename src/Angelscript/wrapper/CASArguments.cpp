#include "angelscript.h"

#ifdef GetObject
#undef GetObject
#endif

#include <cstdarg>

#include "Angelscript/util/ASUtil.h"

#include "CASArguments.h"

#if 0

/*
* CASArgument
*/
CASArgument::~CASArgument()
{
	Reset();
}

CASArgument::CASArgument( const CASArgument& other )
	: m_iTypeId( -1 )
	, m_ArgType( AT_NONE )
	, m_Value()
{
	Set( other );
}

CASArgument& CASArgument::operator=( const CASArgument& other )
{
	if( this != &other )
		Set( other );

	return *this;
}

void* CASArgument::GetArgumentAsPointer() const
{
	if( !HasValue() )
		return nullptr;

	switch( m_ArgType )
	{
	case AT_PRIMITIVE:
	case AT_ENUM:		return const_cast<asQWORD*>( &m_Value.qword );

	case AT_REF:
	case AT_VALUE:		return m_Value.pValue;

	default:			return nullptr;
	}
}

bool CASArgument::Set( int iTypeId, ArgumentType_t type, const ArgumentValue_t& value, bool fCopy )
{
	Reset();

	if( type == AT_NONE )
		return true;

	bool fSuccess = true;

	m_iTypeId = iTypeId;
	m_ArgType = type;

	if( fCopy )
	{
		if( type == AT_VALUE || type == AT_REF )
		{
			asIScriptEngine* pEngine = gASManager()->GetScriptEngine();

			asITypeInfo* pType = pEngine->GetObjectTypeById( iTypeId );

			if( pType )
			{
				//Need to copy value
				if( type == AT_VALUE )
					m_Value.pValue = pEngine->CreateScriptObjectCopy( value.pValue, pType );
				else //Need to addref
				{
					pEngine->AddRefScriptObject( value.pValue, pType );
					m_Value = value;
				}
			}
			else
			{
				gASLog()->Error( ASLOG_CRITICAL, "CASArgument::Set: failed to get object type!\n" );
				fSuccess = false;
			}
		}
		else if( type == AT_VOID )
		{
			//Do nothing
		}
		else //Primitive type or enum, just copy
			m_Value = value;
	}
	else if( type != AT_VOID )
		m_Value = value;

	return fSuccess;
}

bool CASArgument::Set( const CASArgument& other )
{
	return Set( other.GetTypeId(), other.GetArgumentType(), other.GetArgumentValue(), true );
}

void CASArgument::Reset()
{
	if( HasValue() )
	{
		//Release reference if needed
		if( !as::IsPrimitive( m_iTypeId ) && !as::IsEnum( m_iTypeId ) )
		{
			asIScriptEngine* pEngine = gASManager()->GetScriptEngine();
			asITypeInfo* pType = pEngine->GetTypeInfoById( m_iTypeId );

			if( pType )
				pEngine->ReleaseScriptObject( m_Value.pValue, pType );
			else
				gASLog()->Error( ASLOG_CRITICAL, "CASArgument::Reset: failed to get object type!\n" );
		}

		m_iTypeId = -1;
		m_ArgType = AT_NONE;
		m_Value = ArgumentValue_t();
	}
}

/*
* CASArguments
*/
CASArguments::CASArguments()
	: m_pArguments( nullptr )
	, m_uiCount( 0 )
{
}

CASArguments::CASArguments( asIScriptGeneric* pArguments, size_t uiStartIndex )
	: m_pArguments( nullptr )
	, m_uiCount( 0 )
{
	SetArguments( pArguments, uiStartIndex );
}

CASArguments::CASArguments( asIScriptFunction* pTargetFunc, va_list list )
	: m_pArguments( nullptr )
	, m_uiCount( 0 )
{
	SetArguments( pTargetFunc, list );
}

CASArguments::~CASArguments()
{
	Clear();
}

void CASArguments::Release() const
{
	if( InternalRelease() )
		delete this;
}

CASArguments::CASArguments( const CASArguments& other )
	: m_pArguments( nullptr )
	, m_uiCount( 0 )
{
	Assign( other );
}

CASArguments& CASArguments::operator=( const CASArguments& other )
{
	Assign( other );

	return *this;
}

void CASArguments::Assign( const CASArguments& other )
{
	if( this != &other )
	{
		Clear();

		m_uiCount = other.GetCount();

		if( other.HasArguments() )
		{
			m_pArguments = new CASArgument[ m_uiCount ];

			CASArgument* pSourceArgs = other.GetArgumentList();

			//Failure is unlikely, but there might be issues copying between lists that don't occur in init from scripts
			bool fFailure = false;

			asIScriptEngine* pEngine = gASManager()->GetScriptEngine();

			for( size_t uiIndex = 0; uiIndex < m_uiCount && !fFailure; ++uiIndex )
			{
				CASArgument& targetArg = m_pArguments[ uiIndex ];
				CASArgument* pSourceArg = &pSourceArgs[ uiIndex ];

				bool fWasPrimitive = false;

				fFailure = CASReflection::SetPrimitiveArgument( targetArg, pSourceArg->GetTypeId(), &pSourceArg->GetArgumentValue().qword, fWasPrimitive );

				if( !fFailure && !fWasPrimitive )
					fFailure = CASReflection::SetObjectArgument( pEngine, targetArg, pSourceArg->GetTypeId(), &pSourceArg->GetArgumentValue().pValue );
			}

			if( fFailure )
				Clear();
		}
	}
}

void CASArguments::Clear()
{
	if( m_pArguments )
	{
		delete[] m_pArguments;
		m_pArguments = nullptr;
		m_uiCount = 0;
	}
}

bool CASArguments::SetArguments( asIScriptGeneric* pArguments, size_t uiStartIndex )
{
	if( !pArguments )
	{
		gASLog()->Error( ASLOG_CRITICAL, "CASArguments::SetArguments: null arguments pointer!\n" );
		return false;
	}

	const size_t uiArgCount = static_cast<size_t>( pArguments->GetArgCount() );

	//If true, an internal error occured
	if( uiStartIndex > uiArgCount )
	{
		gASLog()->Error( ASLOG_CRITICAL, "CASArguments::SetArguments: start index is greater than argument count!\n" );
		return false;
	}

	const asUINT uiTargetArgs = uiArgCount - uiStartIndex;

	bool fSuccess = true;

	CASArgument* pArgsArr = uiTargetArgs > 0 ? new CASArgument[ uiTargetArgs ] : NULL;

	asIScriptEngine* pEngine = gASManager()->GetScriptEngine();

	for( asUINT uiIndex = 0; uiIndex < uiTargetArgs && fSuccess; ++uiIndex )
	{
		int iTypeId = pArguments->GetArgTypeId( uiIndex + uiStartIndex );
		void* pData = pArguments->GetArgAddress( uiIndex + uiStartIndex );

		fSuccess = CASReflection::SetArgument( pEngine, pArgsArr[ uiIndex ], iTypeId, pData );
	}

	if( fSuccess )
	{
		m_pArguments = pArgsArr;
		m_uiCount = uiTargetArgs;
	}
	else
		delete[] pArgsArr;

	return fSuccess;
}

//Store arguments in this object
bool CASArguments::SetArguments( asIScriptFunction* pTargetFunc, va_list list )
{
	if( !pTargetFunc || !list )
		return false;

	Clear();

	const asUINT uiArgCount = pTargetFunc->GetParamCount();

	CASArgument* pArgsArr = uiArgCount > 0 ? new CASArgument[ uiArgCount ] : NULL;

	bool fSuccess = true;

	asIScriptEngine* pEngine = gASManager()->GetScriptEngine();

	int iTypeId;
	asDWORD uiFlags;

	ArgumentValue_t value;

	for( asUINT uiIndex = 0; uiIndex < uiArgCount && fSuccess; ++uiIndex )
	{
		if( pTargetFunc->GetParam( uiIndex, &iTypeId, &uiFlags ) < 0 )
		{
			gASLog()->Error( ASLOG_CRITICAL, "CASArguments::SetArguments(va_list): an error occurred while getting function parameter information, aborting!\n" );
			fSuccess = false;
			break;
		}

		asITypeInfo* pType = pEngine->GetObjectTypeById( iTypeId );

		asDWORD uiObjFlags = pType->GetFlags();
		ArgumentType_t argType;

		if( ( fSuccess = CASReflection::GetArgumentFromVarargs( value, iTypeId, uiFlags, list, &uiObjFlags, &argType ) ) )
		{
			//Make copies of the input arguments; they won't exist anymore once this method has finished execution.
			pArgsArr[ uiIndex ].Set( iTypeId, argType, value, true );
		}
		else
			gASLog()->Error( ASLOG_CRITICAL, "CASArguments::SetArguments(va_list): Function '%s::s': failed to set argument %u, aborting!\n", 
				pTargetFunc->GetNamespace(), pTargetFunc->GetName(), uiIndex );
	}

	if( fSuccess )
	{
		m_pArguments = pArgsArr;
		m_uiCount = static_cast<size_t>( uiArgCount );
	}
	else
		delete[] pArgsArr;

	return fSuccess;
}

#endif