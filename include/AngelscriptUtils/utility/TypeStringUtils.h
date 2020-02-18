#pragma once

#include <string>

#include <angelscript.h>

class CASArguments;

namespace asutils
{
/**
*	@brief Returns a string representation of a primitive type id
*	@return String representation, or an empty string if the type id is not a primitive type
*/
std::string PrimitiveTypeIdToString(const int typeId);

/**
*	@brief Converts a primitive type to its string representation
*	@param object pointer to primitive value
*/
std::string PrimitiveValueToString(const void* object, const int typeId);

/**
*	@brief Formats a function name and returns it
*	The format for global functions is \<namespace>::\<name>
*	The format for member functions is \<namespace>::\<classname>::\<name>
*	@param function Function whose name should be formatted
*	@return Formatted function name
*/
std::string FormatFunctionName(const asIScriptFunction& function);

/**
*	@brief Printf function used by script functions
*	@param format Format string
*	@param firstIndex Index of the first parameter to use
*	@param arguments Generic arguments instance
*	@return Formatted string
*/
std::string Format(const std::string& format, const asUINT firstIndex, asIScriptGeneric& arguments);

/**
*	@brief Extracts a namespace from a name
*	Namespaces are denoted by double colons. For example "String::EMPTY_STRING"
*	@return If a namespace is contained in the name, returns a string containing that namespace. Otherwise, returns an empty string
*/
inline std::string ExtractNamespaceFromName(const std::string& name)
{
	if (name.empty())
	{
		return {};
	}

	const auto index = name.rfind("::");

	if (index == std::string::npos)
	{
		return {};
	}

	return name.substr(0, index);
}

/**
*	@brief Extracts a name from a name that may contain a namespace
*	Namespaces are denoted by double colons. For example "String::EMPTY_STRING"
*	@return If a name is contained in the name, returns a string containing that name. Otherwise, returns an empty string
*/
inline std::string ExtractNameFromName(const std::string& name)
{
	if (name.empty())
	{
		return {};
	}

	size_t index = name.rfind("::");

	if (index == std::string::npos)
	{
		return name;
	}

	return name.substr(index + 2);
}

/**
*	@brief Extracts a namespace from a declaration
*	Namespaces are denoted by double colons. For example "void String::Compare(const string& in lhs, const string& in rhs)"
*	@param isFunctionDeclaration Whether this is a function or a class declaration
*	@return If a namespace is contained in the declaration, returns a string containing that namespace. Otherwise, returns an empty string
*/
std::string ExtractNamespaceFromDecl(const std::string& declaration, const bool isFunctionDeclaration = true);

/**
*	@brief Creates a function signature that can be called with the given arguments
*	@param function Stream that will receive the signature
*	@param args Argument list
*	@param startIndex First argument in the generic call instance to check
*	@param arguments Generic call instance whose arguments will be used for type information
*	@return true on success, false otherwise
*/
bool CreateFunctionSignature(
	asIScriptEngine& engine,
	std::stringstream& function, const std::string& returnType, const std::string& functionName,
	const CASArguments& args,
	const asUINT startIndex, asIScriptGeneric& arguments);

std::string FormatObjectTypeName(const std::string& scriptNamespace, const std::string& scriptName);
}
