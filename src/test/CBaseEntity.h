#ifndef TEST_CBASEENTITY_H
#define TEST_CBASEENTITY_H

#include <string>

#include "Angelscript/util/Platform.h"

class CBaseEntity
{
public:

	virtual void Spawn()
	{
	}

	virtual int ScheduleOfType( const std::string& ASUNREFERENCED( szName ) )
	{
		return 1;
	}
};

#endif //TEST_CBASEENTITY_H