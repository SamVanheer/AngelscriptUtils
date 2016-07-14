#include <cassert>

#include <type_traits>

#include "CASContext.h"

#include "Angelscript/util/ContextUtils.h"

#include "ASCallable.h"

/**
*	Calls a function.
*	@param callable Callable object.
*	@param flags Call flags.
*	@param args The arguments for the function.
*	@tparam CALLABLE Type of the callable object.
*	@tparam ARGS Arguments list type.
*	@return true on success, false otherwise.
*/
template<typename CALLABLE, typename ARGS>
bool CallFunction( CALLABLE& callable, CallFlags_t flags, const ARGS& args )
{
	auto pContext = callable.GetContext().GetContext();

	//This will trigger an error if we miss a cast.
	static_assert( std::is_same<CALLABLE, CASFunction>::value || std::is_same<CALLABLE, CASMethod>::value, "Must use either CASFunction or CASMethod for this function!" );

	assert( pContext );

	if( !pContext )
		return false;

	auto& function = callable.GetFunction();

	auto result = pContext->Prepare( &function );

	if( result < 0 )
	{
		return false;
	}

	if( !callable.PreSetArguments() )
		return false;

	auto success = ctx::SetArguments( function, *pContext, args );

	if( !success )
		return false;

	if( !callable.PreExecute() )
		return false;

	result = pContext->Execute();

	if( !callable.PostExecute( result ) )
		return false;

	//TODO: check for errors. - Solokiller

	return result >= 0;
}

CASCallable::CASCallable( asIScriptFunction& function, CASContext& context )
	: m_Function( function )
	, m_Context( context )
{
}

bool CASCallable::IsValid() const
{
	return m_Context.GetContext() != nullptr;
}

bool CASCallable::GetReturnValue( void* pReturnValue )
{
	assert( m_Context );

	asDWORD uiFlags;
	int iTypeId = m_Function.GetReturnTypeId( &uiFlags );

	return ctx::GetReturnValue( *m_Context.GetContext(), iTypeId, uiFlags, pReturnValue );
}

CASMethod::CASMethod( asIScriptFunction& function, CASContext& context, void* pThis )
	: CASTCallable( function, context )
	, m_pThis( pThis )
{
	assert( pThis );
}

bool CASMethod::IsValid() const
{
	return CASCallable::IsValid() && m_pThis;
}

bool CASMethod::PreSetArguments()
{
	auto pContext = GetContext().GetContext();

	return pContext->SetObject( m_pThis ) >= 0;
}

namespace as
{
bool VCallFunction( asIScriptFunction* pFunction, asIScriptContext* pContext, CallFlags_t flags, va_list list )
{
	assert( pFunction );
	assert( pContext );

	if( !pFunction )
		return false;

	if( !pContext )
		return false;

	CASContext ctx( *pContext );

	CASFunction func( *pFunction, ctx );

	if( !func.IsValid() )
		return false;

	return func.VCall( flags, list );
}

bool VCallFunction( asIScriptFunction* pFunction, asIScriptContext* pContext, va_list list )
{
	return VCallFunction( pFunction, pContext, CallFlag::NONE, list );
}

bool VCallFunction( asIScriptFunction* pFunction, CallFlags_t flags, va_list list )
{
	assert( pFunction );

	if( !pFunction )
		return false;

	CASOwningContext ctx( *pFunction->GetEngine() );

	if( !ctx )
		return false;

	return VCallFunction( pFunction, ctx.GetContext(), flags, list );
}

bool VCallFunction( asIScriptFunction* pFunction, va_list list )
{
	assert( pFunction );

	if( !pFunction )
		return false;

	CASOwningContext ctx( *pFunction->GetEngine() );

	if( !ctx )
		return false;

	return VCallFunction( pFunction, ctx.GetContext(), list );
}

bool CallFunction( asIScriptFunction* pFunction, asIScriptContext* pContext, CallFlags_t flags, ... )
{
	va_list list;

	va_start( list, flags );

	auto success = VCallFunction( pFunction, pContext, flags, list );

	va_end( list );

	return success;
}

bool CallFunction( asIScriptFunction* pFunction, asIScriptContext* pContext, ... )
{
	assert( pContext );

	if( !pContext )
		return false;

	va_list list;

	va_start( list, pContext );

	auto success = VCallFunction( pFunction, pContext, list );

	va_end( list );

	return success;
}

bool CallFunction( asIScriptFunction* pFunction, CallFlags_t flags, ... )
{
	va_list list;

	va_start( list, flags );

	auto success = VCallFunction( pFunction, flags, list );

	va_end( list );

	return success;
}

bool CallFunction( asIScriptFunction* pFunction, ... )
{
	assert( pFunction );

	if( !pFunction )
		return false;

	va_list list;

	va_start( list, pFunction );

	auto success = VCallFunction( pFunction, list );

	va_end( list );

	return success;
}

bool CallFunctionArgs( asIScriptFunction* pFunction, asIScriptContext* pContext, CallFlags_t flags, const CASArguments& args )
{
	assert( pFunction );
	assert( pContext );

	if( !pFunction )
		return false;

	if( !pContext )
		return false;

	CASContext ctx( *pContext );

	CASFunction func( *pFunction, ctx );

	if( !func.IsValid() )
		return false;

	return func.CallArgs( flags, args );
}

bool CallFunctionArgs( asIScriptFunction* pFunction, asIScriptContext* pContext, const CASArguments& args )
{
	return CallFunctionArgs( pFunction, pContext, CallFlag::NONE, args );
}

bool CallFunctionArgs( asIScriptFunction* pFunction, CallFlags_t flags, const CASArguments& args )
{
	assert( pFunction );

	if( !pFunction )
		return false;

	CASOwningContext ctx( *pFunction->GetEngine() );

	if( !ctx )
		return false;

	return CallFunctionArgs( pFunction, ctx.GetContext(), flags, args );
}

bool CallFunctionArgs( asIScriptFunction* pFunction, const CASArguments& args )
{
	return CallFunctionArgs( pFunction, CallFlag::NONE, args );
}

bool VCallMethod( void* pThis, asIScriptFunction* pFunction, asIScriptContext* pContext, CallFlags_t flags, va_list list )
{
	assert( pFunction );
	assert( pContext );

	if( !pFunction )
		return false;

	if( !pContext )
		return false;

	CASContext ctx( *pContext );

	CASMethod func( *pFunction, ctx, pThis );

	if( !func.IsValid() )
	{
		return false;
	}

	return func.VCall( flags, list );
}

bool VCallMethod( void* pThis, asIScriptFunction* pFunction, asIScriptContext* pContext, va_list list )
{
	return VCallMethod( pThis, pFunction, pContext, CallFlag::NONE, list );
}

bool VCallMethod( void* pThis, asIScriptFunction* pFunction, CallFlags_t flags, va_list list )
{
	assert( pFunction );

	if( !pFunction )
		return false;

	CASOwningContext ctx( *pFunction->GetEngine() );

	if( !ctx )
		return false;

	return VCallMethod( pThis, pFunction, ctx.GetContext(), flags, list );
}

bool VCallMethod( void* pThis, asIScriptFunction* pFunction, va_list list )
{
	assert( pFunction );

	if( !pFunction )
		return false;

	CASOwningContext ctx( *pFunction->GetEngine() );

	if( !ctx )
		return false;

	return VCallMethod( pThis, pFunction, ctx.GetContext(), list );
}

bool CallMethod( void* pThis, asIScriptFunction* pFunction, asIScriptContext* pContext, CallFlags_t flags, ... )
{
	va_list list;

	va_start( list, flags );

	auto success = VCallMethod( pThis, pFunction, pContext, flags, list );

	va_end( list );

	return success;
}

bool CallMethod( void* pThis, asIScriptFunction* pFunction, asIScriptContext* pContext, ... )
{
	assert( pContext );

	if( !pContext )
		return false;

	va_list list;

	va_start( list, pContext );

	auto success = VCallMethod( pThis, pFunction, pContext, list );

	va_end( list );

	return success;
}

bool CallMethod( void* pThis, asIScriptFunction* pFunction, CallFlags_t flags, ... )
{
	va_list list;

	va_start( list, flags );

	auto success = VCallMethod( pThis, pFunction, flags, list );

	va_end( list );

	return success;
}

bool CallMethod( void* pThis, asIScriptFunction* pFunction, ... )
{
	assert( pFunction );

	if( !pFunction )
		return false;

	va_list list;

	va_start( list, pFunction );

	auto success = VCallMethod( pThis, pFunction, list );

	va_end( list );

	return success;
}

bool CallMethodArgs( void* pThis, asIScriptFunction* pFunction, asIScriptContext* pContext, CallFlags_t flags, const CASArguments& args )
{
	assert( pFunction );
	assert( pContext );

	if( !pFunction )
		return false;

	if( !pContext )
		return false;

	CASContext ctx( *pContext );

	CASMethod method( *pFunction, ctx, pThis );

	if( !method.IsValid() )
		return false;

	return method.CallArgs( flags, args );
}

bool CallMethodArgs( void* pThis, asIScriptFunction* pFunction, asIScriptContext* pContext, const CASArguments& args )
{
	return CallMethodArgs( pThis, pFunction, pContext, CallFlag::NONE, args );
}

bool CallMethodArgs( void* pThis, asIScriptFunction* pFunction, CallFlags_t flags, const CASArguments& args )
{
	assert( pFunction );

	if( !pFunction )
		return false;

	CASOwningContext ctx( *pFunction->GetEngine() );

	if( !ctx )
		return false;

	return CallMethodArgs( pThis, pFunction, ctx.GetContext(), flags, args );
}

bool CallMethodArgs( void* pThis, asIScriptFunction* pFunction, const CASArguments& args )
{
	return CallMethodArgs( pThis, pFunction, CallFlag::NONE, args );
}
}