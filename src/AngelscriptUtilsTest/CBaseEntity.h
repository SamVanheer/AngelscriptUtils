#ifndef TEST_CBASEENTITY_H
#define TEST_CBASEENTITY_H

#include <string>

#include "AngelscriptUtils/util/ASPlatform.h"

class CBaseEntity
{
public:

	virtual void Spawn()
	{
	}

	virtual int ScheduleOfType( const std::string& /*szName*/ )
	{
		return 1;
	}
};

#endif //TEST_CBASEENTITY_H