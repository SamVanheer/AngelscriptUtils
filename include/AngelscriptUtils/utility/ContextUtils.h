#pragma once

#include <string>

#include <angelscript.h>

namespace asutils
{
struct LocationInfo
{
	std::string section;
	int line = 0;
	int column = 0;
};

/**
*	@brief Gets information about the calling script
*	@param context Context to retrieve the info from
*	@param[ out ] section Name of the section, or "Unknown" if it couldn't be retrieved
*	@param[ out ] line Line number, or 0 if it couldn't be retrieved
*	@param[ out ] column Column, or 0 if it couldn't be retrieved
*	@return Whether the caller info is valid
*/
inline bool GetCallerInfo(asIScriptContext& context, std::string& section, int& line, int& column)
{
	const char* sectionName = nullptr;

	line = context.GetLineNumber(0, &column, &sectionName);

	if (!sectionName)
	{
		sectionName = "Unknown";
	}

	section = sectionName;

	return line >= 0;
}

/**
*	@brief Gets information about the calling script
*	@param context Context to retrieve the info from
*	@param[ out ] info Caller info
*	@return Whether the caller info is valid
*/
inline bool GetCallerInfo(asIScriptContext& context, LocationInfo& info)
{
	return GetCallerInfo(context, info.section, info.line, info.column);
}

inline bool GetExceptionInfo(asIScriptContext& context, LocationInfo& info)
{
	if (context.GetState() == asEXECUTION_EXCEPTION)
	{
		const char* sectionName = nullptr;

		info.line = context.GetExceptionLineNumber(&info.column, &sectionName);

		if (!sectionName)
		{
			sectionName = "Unknown";
		}

		info.section = sectionName;

		return true;
	}
	else
	{
		info.section = "No exception";
		info.line = 0;
		info.column = 0;

		return false;
	}
}
}
