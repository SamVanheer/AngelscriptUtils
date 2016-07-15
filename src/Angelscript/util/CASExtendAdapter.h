#ifndef ANGELSCRIPT_UTIL_CASEXTENDADAPTER_H
#define ANGELSCRIPT_UTIL_CASEXTENDADAPTER_H

#include <cassert>

#include "IASExtendAdapter.h"

#include "CASObjPtr.h"

#include "Angelscript/wrapper/ASCallable.h"

/**
*	@addtogroup ASExtend
*
*	@{
*/

/**
*	Class used to adapt C++ classes for extension in Angelscript.
*	@tparam BASECLASS C++ class to extend. Should be a class with at least one virtual function.
*	@tparam DERIVEDCLASS C++ class that will be instantiated. Used to call object methods correctly.
*/
template<typename BASECLASS, typename DERIVEDCLASS>
class CASExtendAdapter : public BASECLASS, public IASExtendAdapter
{
public:
	typedef BASECLASS BaseClass;
	typedef CASExtendAdapter ThisClass;
	typedef DERIVEDCLASS DerivedClass;

public:
	/**
	*	Constructor.
	*	@param object Script object that extends this object.
	*/
	CASExtendAdapter( CASObjPtr object )
		: BASECLASS()
		, m_Object( object )
	{
		assert( object );
	}

	/**
	*	Constructor.
	*	@param object Script object that extends this object.
	*	@param args Arguments for the baseclass constructor.
	*	@tparam ARGS Argument types for the baseclass constructor.
	*/
	template<typename... ARGS>
	CASExtendAdapter( CASObjPtr object, ARGS&&... args )
		: BASECLASS( std::move( args ) )
		, m_Object( object )
	{
		assert( object );
	}

	/**
	*	@return The object instance.
	*/
	const CASObjPtr& GetObject() const override final
	{
		return m_Object;
	}

	/**
	*	@copydoc GetObject() const
	*/
	CASObjPtr GetObject() override final
	{
		return m_Object;
	}

	/**
	*	Calls a method.
	*	@param pszDecl Method declaration.
	*	@param func C++ function to call if the method isn't found in the script.
	*	@param args Arguments to pass to either method.
	*	@tparam FUNC Function pointer type.
	*	@tparam ARGS Argument types.
	*/
	template<typename FUNC, typename... ARGS>
	void Call( const char* const pszDecl, FUNC func, ARGS&&... args )
	{
		if( auto pFunction = GetObject().GetTypeInfo()->GetMethodByDecl( pszDecl ) )
		{
			as::Call( GetObject().Get(), pFunction, std::move( args )... );
		}
		else
		{
			( *static_cast<DerivedClass*>( this ).*func )( std::move( args )... );
		}
	}

	/**
	*	Calls a method.
	*	@param result Result value.
	*	@param pszDecl Method declaration.
	*	@param func C++ function to call if the method isn't found in the script.
	*	@param args Arguments to pass to either method.
	*	@tparam RESULT Result type.
	*	@tparam FUNC Function pointer type.
	*	@tparam ARGS Argument types.
	*/
	template<typename RESULT, typename FUNC, typename... ARGS>
	void Call( RESULT& result, const char* const pszDecl, FUNC func, ARGS&&... args )
	{
		if( auto pFunction = GetObject().GetTypeInfo()->GetMethodByDecl( pszDecl ) )
		{
			CASOwningContext ctx( *pFunction->GetEngine() );

			CASMethod method( *pFunction, ctx, GetObject().Get() );

			if( method.Call( CallFlag::NONE, std::move( args )... ) )
			{
				method.GetReturnValue( &result );
			}
		}
		else
		{
			result = ( *static_cast<DerivedClass*>( this ).*func )( std::move( args )... );
		}
	}

private:
	CASObjPtr m_Object;

private:
	//TODO: consider adding support for copying.
	CASExtendAdapter( const CASExtendAdapter& ) = delete;
	CASExtendAdapter& operator=( const CASExtendAdapter& ) = delete;
};

/*
*	Helper macros.
*/

/**
*	Calls a method that has a different C++ method name, and returns non-void.
*	@param retType Return type.
*	@param methodName Script method name to call.
*	@param baseMethodName C++ method name to call.
*	@param pszParams Script method parameters.
*	@param ... Arguments to pass.
*/
#define CALL_EXTEND_FUNC_RET_DIFFFUNC( retType, methodName, baseMethodName, pszParams, ... )			\
retType result;																							\
																										\
Call( result, #retType " " #methodName "(" pszParams " )", &ThisClass::baseMethodName, __VA_ARGS__ );	\
																										\
return result

/**
*	Calls a method, and returns non-void.
*	@param retType Return type.
*	@param methodName Script or C++ method name to call.
*	@param pszParams Script method parameters.
*	@param ... Arguments to pass.
*/
#define CALL_EXTEND_FUNC_RET( retType, methodName, pszParams, ... )							\
CALL_EXTEND_FUNC_RET_DIFFFUNC( retType, methodName, methodName, pszParams, __VA_ARGS__ )

/**
*	Calls a method that has a different C++ method name.
*	@param methodName Script method name to call.
*	@param baseMethodName C++ method name to call.
*	@param pszParams Script method parameters.
*	@param ... Arguments to pass.
*/
#define CALL_EXTEND_FUNC_DIFFFUNC( methodName, baseMethodName, pszParams, ... )			\
Call( "void " #methodName "(" pszParams " )", &ThisClass::baseMethodName, __VA_ARGS__ )

/**
*	Calls a method.
*	@param methodName Script or C++ method name to call.
*	@param pszParams Script method parameters.
*	@param ... Arguments to pass.
*/
#define CALL_EXTEND_FUNC( methodName, pszParams, ... )						\
CALL_EXTEND_FUNC_DIFFFUNC( methodName, methodName, pszParams, __VA_ARGS__ )

/** @} */

#endif //ANGELSCRIPT_UTIL_CASEXTENDADAPTER_H