#pragma once

#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <angelscript.h>

namespace asutils
{
/**
*	@brief Provides a means of tracking all global variables that are to be added to a script and initializing them after script compilation
*/
class GlobalVariablesList final
{
public:
	using Initializer = std::function<bool(const std::string & typeName, const std::string & variableName, asIScriptModule& module, void* userData)>;

private:
	struct GlobalVariable
	{
		GlobalVariable(std::string&& typeName, std::string&& variableName, Initializer&& initializer)
			: TypeName(std::move(typeName))
			, VariableName(std::move(variableName))
			, Initializer(std::move(initializer))
		{
		}

		const std::string TypeName;
		const std::string VariableName;
		const Initializer Initializer;
	};

public:
	GlobalVariablesList() = default;
	~GlobalVariablesList() = default;

	GlobalVariablesList(const GlobalVariablesList&) = delete;
	GlobalVariablesList& operator=(const GlobalVariablesList&) = delete;

	/**
	*	@brief Adds a new global variable to the list
	*	@param typeName The type of the global variable, including any modifiers such as const and/or '@'
	*	@param variableName Name of the variable. Variable names are unique
	*	@param initializer Function that will be invoked to initialize the global variable
	*	@exception std::invalid_argument If the type name is empty or
	*		If the variable name is empty or
	*		If the initializer is null or
	*		If the variable name is taken by another registered variable
	*/
	void Add(std::string&& typeName, std::string&& variableName, Initializer&& initializer)
	{
		if (typeName.empty())
		{
			throw std::invalid_argument("Type name must be valid");
		}

		if (variableName.empty())
		{
			throw std::invalid_argument("Variable name must be valid");
		}

		if (!initializer)
		{
			throw std::invalid_argument("Initializer must be valid");
		}

		if (m_Globals.find(variableName) != m_Globals.end())
		{
			throw std::invalid_argument("Global variable \"" + FormatVariableDeclaration(typeName, variableName) + "\" already added");
		}

		auto variable = std::make_unique<GlobalVariable>(std::move(typeName), std::move(variableName), std::move(initializer));

		m_Globals.emplace(variable->VariableName, std::move(variable));
	}

	/**
	*	@brief Returns a string containing all currently declared globals as a single source section, ready to be added to a module
	*/
	std::string GetDeclarationsAsSection() const
	{
		std::ostringstream buffer;

		for (const auto& variable : m_Globals)
		{
			buffer << FormatVariableDeclaration(variable.second->TypeName, variable.second->VariableName) << '\n';
		}

		return buffer.str();
	}

	/**
	*	@brief Initializes all global variables in the given module
	*	@param module Module to initialize
	*	@param userData Optional user data to pass to the initializer
	*	@return Whether all initializers executed successfully
	*/
	bool InitializeAll(asIScriptModule& module, void* userData = nullptr)
	{
		bool allSucceeded = true;

		for (const auto& pair : m_Globals)
		{
			const auto& variable = *pair.second;
			allSucceeded = variable.Initializer(variable.TypeName, variable.VariableName, module, userData) && allSucceeded;
		}

		return allSucceeded;
	}

private:
	static std::string FormatVariableDeclaration(const std::string& typeName, const std::string& variableName)
	{
		return typeName + " " + variableName + ";";
	}

private:
	std::unordered_map<std::string, std::unique_ptr<GlobalVariable>> m_Globals;
};
}
