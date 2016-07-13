#ifndef UTIL_ASUTIL_H
#define UTIL_ASUTIL_H

#include <angelscript.h>

class CScriptAny;

/**
*	@defgroup ASUtil Angelscript Utility Code
*
*	@{
*/

namespace as
{
/**
*	Returns a string representation of a primitive type id.
*	@param iTypeId Type Id.
*	@return String representation, or null if the type id is not a primitive type.
*/
const char* PrimitiveTypeIdToString( const int iTypeId );

/**
*	Returns whether the given type id is that of void.
*	Exists mostly for completeness.
*	@param iTypeId Type Id.
*	@return true if it is void, false otherwise.
*/
inline constexpr bool IsVoid( const int iTypeId )
{
	return asTYPEID_VOID == iTypeId;
}

/**
*	Returns whether a given type id is that of a primitive type.
*	@param iTypeId Type Id.
*	@return true if it is a primitive type, false otherwise.
*/
inline constexpr bool IsPrimitive( const int iTypeId )
{
	return asTYPEID_BOOL <= iTypeId && iTypeId <= asTYPEID_DOUBLE;
}

/**
*	Returns whether a given type id is that of an enum type.
*	@param iTypeId Type Id.
*	@return true if it is an enum type, false otherwise.
*/
inline constexpr bool IsEnum( const int iTypeId )
{
	return ( iTypeId > asTYPEID_DOUBLE && ( iTypeId & asTYPEID_MASK_OBJECT ) == 0 );
}

/**
*	Returns whether a given type id is that of an integer type.
*	@param iTypeId Type Id.
*	@return true if it is an integer type, false otherwise.
*/
inline constexpr bool IsInteger( const int iTypeId )
{
	return ( ( iTypeId >= asTYPEID_INT8 ) && ( iTypeId <= asTYPEID_UINT64 ) );
}

/**
*	Returns whether a given type id is that of a float type.
*	@param iTypeId Type Id.
*	@return true if it is a float type, false otherwise.
*/
inline constexpr bool IsFloat( const int iTypeId )
{
	return ( ( iTypeId >= asTYPEID_FLOAT ) && ( iTypeId <= asTYPEID_DOUBLE ) );
}

/**
*	Returns whether a given type id is that of a primitive type.
*	@param iTypeId Type Id.
*	@return true if it is a primitive type, false otherwise.
*/
inline constexpr bool IsObject( const int iTypeId )
{
	return ( iTypeId & asTYPEID_MASK_OBJECT ) != 0;
}

/**
*	Releases a vararg argument.
*	@param engine Script engine.
*	@param pObject Object pointer.
*	@param iTypeId Type Id.
*/
void ReleaseVarArg( asIScriptEngine& engine, void* pObject, const int iTypeId );

/**
*	@param type Object type.
*	@return Whether the given type has a default constructor.
*/
bool HasDefaultConstructor( const asITypeInfo& type );

/**
*	Creates an instance of an object using its default constructor.
*	@param type Object type.
*	@return Object instance, or null if the object could not be instantiated.
*/
void* CreateObjectInstance( asIScriptEngine& engine, const asITypeInfo& type );

/**
*	Creates an any object that converts integer and float types smaller than 64 bit to their 64 bit type so any::retrieve works correctly.
*	Angelscript will select the int64 or double overload when any integer or float type is passed, so this is necessary.
*	@param engine Script engine.
*	@param pObject Object to set in the any.
*	@param iTypeId Type Id.
*	@return Any instance.
*/
CScriptAny* CreateScriptAny( asIScriptEngine& engine, void* pObject, int iTypeId );

/**
*	Converts a primitive type to its string representation.
*	@param pszBuffer Output buffer.
*	@param uiBufferSize Buffer size, in characters.
*	@param pObject pointer to primitive value.
*	@param iTypeId Type Id.
*	@return true on success, false otherwise;
*/
bool PODToString( char* pszBuffer, const size_t uiBufferSize, const void* pObject, const int iTypeId );

/**
*	Printf function used by script functions.
*	@param pszBuffer Output buffer.
*	@param uiBufferSize Buffer size, in characters.
*	@param pszFormat Format string.
*	@param uiFirstParamIndex Index of the first parameter to use.
*	@param arguments Generic arguments instance.
*/
bool SPrintf( char* pszBuffer, const size_t uiBufferSize, const char* pszFormat, size_t uiFirstParamIndex, asIScriptGeneric& arguments );

/**
*	Overload that determines buffer size automatically.
*	@see SPrintf( char* pszBuffer, const size_t uiBufferSize, const char* pszFormat, size_t uiFirstParamIndex, asIScriptGeneric& arguments )
*/
template<size_t BUFFER_SIZE>
bool SPrintf( char( &szBuffer )[ BUFFER_SIZE ], const char* pszFormat, size_t uiFirstParamIndex, asIScriptGeneric& arguments )
{
	return SPrintf( szBuffer, BUFFER_SIZE, pszFormat, uiFirstParamIndex, arguments );
}

/**
*	Used to handle the AddRef and Release behaviors on an Angelscript object when using template functions.
*	Specialize it for classes that use different method names.
*/
template<typename T>
class CASReferenceAdapter
{
public:

	static inline void AddRef( const T* const pObj )
	{
		pObj->AddRef();
	}

	static inline void Release( const T* const pObj )
	{
		pObj->Release();
	}

private:
	//Disable this stuff
	CASReferenceAdapter() = delete;
	CASReferenceAdapter( const CASReferenceAdapter& ) = delete;
	CASReferenceAdapter& operator=( const CASReferenceAdapter& ) = delete;
};

/*
*	Handles the setting of a pointer to an Angelscript reference counted object.
*	@param pPointer The pointer to assign the object to.
*	@param pObj Pointer to the object being assigned. Can be null.
*	@param bTransferOwnership If true, calls Release on the original pointer if it is non-null.
*
*	@returnPpointer to the object.
*/
template<typename T>
inline T* ASSetRefPointer( T*& pPointer, T* const pObj, const bool bTransferOwnership = true )
{
	if( pPointer )
	{
		CASReferenceAdapter<T>::Release( pPointer );
	}

	if( pObj )
	{
		pPointer = pObj;

		if( !bTransferOwnership )
			CASReferenceAdapter<T>::AddRef( pPointer );
	}
	else
	{
		pPointer = nullptr;
	}

	return pPointer;
}
}

/** @} */

#endif //UTIL_ASUTIL_H