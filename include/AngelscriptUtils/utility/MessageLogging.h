#pragma once

#include <ostream>
#include <sstream>
#include <string>

#include <angelscript.h>

namespace asutils
{
/**
*	@brief Formats a message and inserts the result into the stream
*/
inline void FormatEngineMessage(const asSMessageInfo& message, std::ostream& stream, const bool appendNewline = true)
{
	const char* type = "";

	//Get the prefix
	switch (message.type)
	{
	case asMSGTYPE_ERROR:
		type = "Error: ";
		break;

	case asMSGTYPE_WARNING:
		type = "Warning: ";
		break;

	default: break;
	}

	//Only display the section if it was actually set. Some messages are not triggered by script code compilation or execution
	const bool hasSection = message.section && *message.section;

	bool needsNewline = false;

	if (hasSection)
	{
		stream << "Section \"" << message.section << "\"";

		needsNewline = true;
	}

	//Some messages don't refer to script code, and set both to 0
	if (message.row != 0 && message.col != 0)
	{
		if (hasSection)
		{
			stream << ' ';
		}

		stream << "(" << message.row << ", " << message.col << ")";

		needsNewline = true;
	}

	if (needsNewline)
	{
		stream << std::endl;
	}

	stream << type << message.message;

	if (appendNewline)
	{
		stream << std::endl;
	}
}

/**
*	@brief Formats a message and returns the result
*/
inline std::string FormatEngineMessage(const asSMessageInfo message, const bool appendNewline = true)
{
	std::ostringstream stream;

	FormatEngineMessage(message, stream, appendNewline);

	return stream.str();
}
}
