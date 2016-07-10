#include <cassert>

#include "CASContext.h"

#include "util/ContextUtils.h"

#include "ASCallable.h"

template<typename CALLABLE>
bool CallFunction( CALLABLE& callable, CallFlags_t flags, va_list list )
{
	auto pContext = callable.GetContext()->GetContext();

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

	auto success = ctx::SetArguments( function, *pContext, list );

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
	, m_pContext( &context )
{
}

bool CASCallable::IsValid() const
{
	return m_pContext && m_pContext->GetContext();
}

bool CASFunction::VCall( CallFlags_t flags, va_list list )
{
	return CallFunction( *this, flags, list );
}

bool CASFunction::operator()( CallFlags_t flags, ... )
{
	va_list list;

	va_start( list, flags );

	const auto success = VCall( flags, list );

	va_end( list );

	return success;
}

bool CASMethod::IsValid() const
{
	return CASCallable::IsValid() && m_pThis;
}

bool CASMethod::VCall( CallFlags_t flags, va_list list )
{
	return CallFunction( *this, flags, list );
}

bool CASMethod::operator()( CallFlags_t flags, ... )
{
	va_list list;

	va_start( list, flags );

	const auto success = VCall( flags, list );

	va_end( list );

	return success;
}

bool CASMethod::PreSetArguments()
{
	auto pContext = GetContext()->GetContext();

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

	CASNonOwningContext ctx( *pContext );

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

	auto pContext = pFunction->GetEngine()->RequestContext();

	if( !pContext )
		return false;

	auto success = VCallFunction( pFunction, pContext, flags, list );

	pFunction->GetEngine()->ReturnContext( pContext );

	return success;
}

bool VCallFunction( asIScriptFunction* pFunction, va_list list )
{
	assert( pFunction );

	if( !pFunction )
		return false;

	auto pContext = pFunction->GetEngine()->RequestContext();

	if( !pContext )
		return false;

	auto success = VCallFunction( pFunction, pContext, list );

	pFunction->GetEngine()->ReturnContext( pContext );

	return success;
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

bool VCallMethod( void* pThis, asIScriptFunction* pFunction, asIScriptContext* pContext, CallFlags_t flags, va_list list )
{
	assert( pFunction );
	assert( pContext );

	if( !pFunction )
		return false;

	if( !pContext )
		return false;

	CASNonOwningContext ctx( *pContext );

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

	auto pContext = pFunction->GetEngine()->RequestContext();

	if( !pContext )
		return false;

	auto success = VCallMethod( pThis, pFunction, pContext, flags, list );

	pFunction->GetEngine()->ReturnContext( pContext );

	return success;
}

bool VCallMethod( void* pThis, asIScriptFunction* pFunction, va_list list )
{
	assert( pFunction );

	if( !pFunction )
		return false;

	auto pContext = pFunction->GetEngine()->RequestContext();

	if( !pContext )
		return false;

	auto success = VCallMethod( pThis, pFunction, pContext, list );

	pFunction->GetEngine()->ReturnContext( pContext );

	return success;
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
}