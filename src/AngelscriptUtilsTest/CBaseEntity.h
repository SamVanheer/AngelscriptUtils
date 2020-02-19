#pragma once

#include <string>

class CBaseEntity
{
public:

	virtual void Spawn()
	{
	}

	virtual int ScheduleOfType(const std::string& /*szName*/)
	{
		return 1;
	}
};
