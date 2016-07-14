#include <type_traits>

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

/*
*	Explicitly instantiate the template variations. Otherwise, there will be linker errors.
*/

template
bool CallFunction<CASFunction, CASArguments>( CASFunction& callable, CallFlags_t flags, const CASArguments& args );

template
bool CallFunction<CASFunction, va_list>( CASFunction& callable, CallFlags_t flags, const va_list& args );

template
bool CallFunction<CASMethod, CASArguments>( CASMethod& callable, CallFlags_t flags, const CASArguments& args );

template
bool CallFunction<CASMethod, va_list>( CASMethod& callable, CallFlags_t flags, const va_list& args );

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