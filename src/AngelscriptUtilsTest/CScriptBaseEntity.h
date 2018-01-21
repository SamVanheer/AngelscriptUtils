#ifndef TEST_CSCRIPTBASEENTITY_H
#define TEST_CSCRIPTBASEENTITY_H

#include "AngelscriptUtils/util/CASExtendAdapter.h"
#include "CBaseEntity.h"

/**
*	Class that bridges the gap between C++ and Angelscript method calls.
*/
class CScriptBaseEntity : public CASExtendAdapter<CBaseEntity, CScriptBaseEntity>
{
public:
	typedef CASExtendAdapter<CBaseEntity, CScriptBaseEntity> BaseClass;
	typedef CScriptBaseEntity ThisClass;

public:
	CScriptBaseEntity( CASObjPtr object )
		: CASExtendAdapter( object )
	{
	}

	void Spawn() override
	{
		CALL_EXTEND_FUNC( Spawn, "()" );
	}

	//This is used to handle the reference type being a pointer.
	int ScheduleOfTypePtr( const std::string* pszName )
	{
		return BaseClass::ScheduleOfType( *pszName );
	}

	int ScheduleOfType( const std::string& szName ) override
	{
		CALL_EXTEND_FUNC_RET_DIFFFUNC( int, ScheduleOfType, ThisClass::ScheduleOfTypePtr, "(const string& in)", &szName );
	}
};

#endif //TEST_CSCRIPTBASEENTITY_H