#ifndef ANGELSCRIPT_CASEVENTCALLER_H
#define ANGELSCRIPT_CASEVENTCALLER_H

#include "CASBaseEventCaller.h"

#include "CASEvent.h"

/**
*	Class that can call CASEvent classes.
*/
class CASEventCaller : public CASBaseEventCaller<CASEventCaller, CASEvent, HookCallResult, HookCallResult::FAILED>
{
public:
	ReturnType_t CallEvent( EventType_t& event, asIScriptContext* pContext, CallFlags_t flags, va_list list );
};

#endif //ANGELSCRIPT_CASEVENTCALLER_H
