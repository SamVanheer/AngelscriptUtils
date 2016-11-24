#include <type_traits>

#include "CASEventCaller.h"

CASEventCaller::ReturnType_t CASEventCaller::CallEvent( EventType_t& event, asIScriptContext* pContext, CallFlags_t flags, va_list list )
{
	assert( pContext );

	if( !pContext )
		return HookCallResult::FAILED;

	CASContext ctx( *pContext );

	bool bSuccess = true;

	HookReturnCode returnCode = HookReturnCode::CONTINUE;

	asIScriptModule* pLastModule = nullptr;

	IncrementCallCount( event );

	const auto functionCount = event.GetFunctionCount();

	const auto stopMode = event.GetStopMode();

	decltype( event.GetFunctionByIndex( 0 ) ) pFunc;

	for( std::remove_const<decltype( functionCount )>::type index = 0; index < functionCount; ++index )
	{
		pFunc = event.GetFunctionByIndex( index );

		if( stopMode == EventStopMode::MODULE_HANDLED && returnCode == HookReturnCode::HANDLED )
		{
			//A hook in the last module handled it, so stop.
			if( pLastModule && pLastModule != pFunc->GetModule() )
				break;
		}

		pLastModule = pFunc->GetModule();

		CASFunction func( *pFunc, ctx );

		const auto successCall = func.VCall( flags, list );

		bSuccess = successCall && bSuccess;

		//Only check if a HANDLED value was returned if we're still continuing.
		if( successCall && returnCode == HookReturnCode::CONTINUE )
		{
			bSuccess = func.GetReturnValue( &returnCode ) && bSuccess;
		}

		if( returnCode == HookReturnCode::HANDLED )
		{
			if( stopMode == EventStopMode::ON_HANDLED )
				break;
		}
	}

	DecrementCallCount( event );

	assert( GetCallCount( event ) >= 0 );

	if( !bSuccess )
		return HookCallResult::FAILED;

	return returnCode == HookReturnCode::HANDLED ? HookCallResult::HANDLED : HookCallResult::NONE_HANDLED;
}
