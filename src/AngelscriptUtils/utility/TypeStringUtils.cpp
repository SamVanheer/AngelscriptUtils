#include <sstream>
#include <stdexcept>

#include "AngelscriptUtils/utility/TypeInfo.h"
#include "AngelscriptUtils/utility/TypeStringUtils.h"

namespace asutils
{
std::string PrimitiveTypeIdToString(int typeId)
{
	switch (typeId)
	{
	case asTYPEID_VOID:		return "void";
	case asTYPEID_BOOL:		return "bool";
	case asTYPEID_INT8:		return "int8";
	case asTYPEID_INT16:	return "int16";
	case asTYPEID_INT32:	return "int32";
	case asTYPEID_INT64:	return "int64";
	case asTYPEID_UINT8:	return "uint8";
	case asTYPEID_UINT16:	return "uint16";
	case asTYPEID_UINT32:	return "uint32";
	case asTYPEID_UINT64:	return "uint64";
	case asTYPEID_FLOAT:	return "float";
	case asTYPEID_DOUBLE:	return "double";
	default:				return {};
	}
}

std::string PrimitiveValueToString(const void* object, const int typeId)
{
	if (!object)
	{
		throw std::invalid_argument("Object must be non-null");
	}

	switch (typeId)
	{
	case asTYPEID_VOID:
	{
		//Treat as null handle
		std::ostringstream stream;
		stream << "0x" << std::hex << static_cast<uintptr_t>(0);
		return stream.str();
	}

	case asTYPEID_BOOL: return *reinterpret_cast<const bool*>(object) != 0 ? "true" : "false";

	case asTYPEID_INT8: return std::to_string(*reinterpret_cast<const char*>(object));
	case asTYPEID_INT16: return std::to_string(*reinterpret_cast<const short*>(object));
	case asTYPEID_INT32: return std::to_string(*reinterpret_cast<const long*>(object));
	case asTYPEID_INT64: return std::to_string(*reinterpret_cast<const long long*>(object));

	case asTYPEID_UINT8: return std::to_string(*reinterpret_cast<const unsigned char*>(object));
	case asTYPEID_UINT16: return std::to_string(*reinterpret_cast<const unsigned short*>(object));
	case asTYPEID_UINT32: return std::to_string(*reinterpret_cast<const unsigned long*>(object));
	case asTYPEID_UINT64: return std::to_string(*reinterpret_cast<const unsigned long long*>(object));

	case asTYPEID_FLOAT: return std::to_string(*reinterpret_cast<const float*>(object));
	case asTYPEID_DOUBLE: return std::to_string(*reinterpret_cast<const double*>(object));

	default: throw std::invalid_argument("Type id must be a primitive type");
	}
}

std::string FormatFunctionName(const asIScriptFunction& function)
{
	auto actualFunction = &function;

	{
		//If this is a delegate, get the original function.
		auto delegate = actualFunction->GetDelegateFunction();

		if (delegate)
		{
			actualFunction = delegate;
		}
	}

	const auto scriptNamespace = actualFunction->GetNamespace();
	const auto objectName = actualFunction->GetObjectName();
	const auto scriptName = actualFunction->GetName();

	const char namespaceSeparator[] = "::";

	std::string name;

	if (scriptNamespace && *scriptNamespace)
	{
		name += scriptNamespace;
		name += namespaceSeparator;
	}

	if (objectName)
	{
		name += objectName;
		name += namespaceSeparator;
	}

	name += scriptName;

	return name;
}

std::string Format(const std::string& format, const asUINT firstIndex, asIScriptGeneric& arguments)
{
	if (format.empty())
	{
		return {};
	}

	const asUINT fullParameterCount = arguments.GetArgCount();

	//TODO: use fmtlib syntax & backend - Solokiller
	if (firstIndex >= fullParameterCount)
	{
		return {};
	}

	//Total number of arguments - offset
	const asUINT parameterCount = fullParameterCount - firstIndex;

	std::ostringstream stream;

	auto& engine = *arguments.GetEngine();

	auto formatPointer = format.c_str();

	//TODO: this should be rewritten to allow for format specifiers.
	while (*formatPointer)
	{
		if (*formatPointer == '%')
		{
			//We've encountered a format parameter
			++formatPointer;

			if (*formatPointer == '%')
			{
				//Just insert a %
				stream << '%';
			}
			else
			{
				//Parse index
				char* endPointer;
				const unsigned int index = strtoul(formatPointer, &endPointer, 10);

				const size_t indexLength = endPointer - formatPointer;

				//If the index is invalid, stop.
				if (index == 0 || index > parameterCount)
				{
					engine.WriteMessage("Format", -1, -1, asMSGTYPE_ERROR, "Format parameter index is out of range");

					return {};
				}
				else
				{
					//Index is 1 based: make 0 based and offset to first actual varargs parameter
					const asUINT parameterIndex = (index - 1) + firstIndex;
					const int typeId = arguments.GetArgTypeId(parameterIndex);

					void* value = arguments.GetArgAddress(parameterIndex);

					if (IsPrimitive(typeId))
					{
						stream << PrimitiveValueToString(value, typeId);
					}
					else
					{
						auto type = engine.GetTypeInfoById(typeId);

						if (type)
						{
							//It's a string
							if (strcmp(type->GetName(), "string") == 0)
							{
								const auto& szString = *reinterpret_cast<const std::string*>(value);

								stream << szString;
							}
							else
							{
								//Pointer types
								stream << "0x" << std::hex << reinterpret_cast<uintptr_t>(value) << std::dec;
							}
						}
						else
						{
							if (value) //Treat as dword
							{
								stream << *reinterpret_cast<asDWORD*>(value);
							}
							else //Treat as pointer
							{
								stream << "0x" << std::hex << reinterpret_cast<uintptr_t>(value) << std::dec;
							}
						}
					}

					formatPointer += indexLength - 1; //Account for the ++ below
				}
			}
		}
		else
		{
			stream << *formatPointer;
		}

		++formatPointer;
	}

	return stream.str();
}

std::string ExtractNamespaceFromDecl(const std::string& declaration, const bool isFunctionDeclaration)
{
	if (declaration.empty())
	{
		return {};
	}

	size_t start;

	bool foundWhitespace = false;

	for (start = 0; start < declaration.length(); ++start)
	{
		if (!foundWhitespace)
		{
			if (isspace(declaration[start]))
			{
				foundWhitespace = true;
			}
		}
		else
		{
			if (!isspace(declaration[start]))
			{
				break;
			}
		}
	}

	if (start >= declaration.length())
	{
		return {};
	}

	size_t end;

	if (isFunctionDeclaration)
	{
		end = declaration.find('(', start + 1);

		if (end == std::string::npos)
		{
			return {};
		}
	}
	else
	{
		end = std::string::npos;
	}

	size_t namespaceEnd = declaration.rfind("::", end);

	if (namespaceEnd == std::string::npos || namespaceEnd <= start)
	{
		return {};
	}

	return declaration.substr(start, namespaceEnd - start);
}

std::string FormatObjectTypeName(const std::string& scriptNamespace, const std::string& scriptName)
{
	if (scriptNamespace.empty())
	{
		return scriptName;
	}

	return scriptNamespace + "::" + scriptName;
}
}
