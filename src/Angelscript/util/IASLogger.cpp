#include <cassert>

#include "IASLogger.h"

namespace ASLog
{
const char* ToString( const ASLog logLevel )
{
	switch( logLevel )
	{
	case CRITICAL:		return "CRITICAL";
	case NORMAL:		return "NORMAL";
	case VERBOSE:		return "VERBOSE";
	case DIAGNOSTIC:	return "DIAGNOSTIC";
	}

	//Lower values are more critical, higher values are more diagnostic.
	if( logLevel < CRITICAL )
		"UNKNOWN CRITICAL";

	return "UNKNOWN DIAGNOSTIC";
}
}