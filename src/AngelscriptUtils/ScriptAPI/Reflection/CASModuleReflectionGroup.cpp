#include "AngelscriptUtils/CASModule.h"

#include "AngelscriptUtils/utility/TypeStringUtils.h"

#include "AngelscriptUtils/ScriptAPI/Reflection/CASModuleReflectionGroup.h"

namespace asutils
{
asIScriptFunction* CASModuleReflectionGroup::FindGlobalFunction(const std::string& name, bool searchByDecl)
{
	auto& scriptModule = *GetScriptModuleFromScriptContext(asGetActiveContext());

	asIScriptFunction* function = nullptr;

	if (searchByDecl)
	{
		const std::string oldNamespace = scriptModule.GetDefaultNamespace();

		const std::string currentNamespace = asutils::ExtractNamespaceFromDecl(name);

		scriptModule.SetDefaultNamespace(currentNamespace.c_str());

		function = scriptModule.GetFunctionByDecl(name.c_str());

		scriptModule.SetDefaultNamespace(oldNamespace.c_str());
	}
	else
	{
		const std::string currentNamespace = asutils::ExtractNamespaceFromName(name);
		const std::string actualName = asutils::ExtractNameFromName(name);

		const asUINT count = scriptModule.GetFunctionCount();

		for (asUINT index = 0; index < count; ++index)
		{
			auto candidateFunction = scriptModule.GetFunctionByIndex(index);

			if (actualName == candidateFunction->GetName() && currentNamespace == candidateFunction->GetNamespace())
			{
				function = candidateFunction;
				break;
			}
		}
	}

	if (function)
	{
		function->AddRef();
	}

	return function;
}

asUINT CASModuleReflectionGroup::GetGlobalFunctionCount() const
{
	auto& scriptModule = *GetScriptModuleFromScriptContext(asGetActiveContext());

	return scriptModule.GetFunctionCount();
}

asIScriptFunction* CASModuleReflectionGroup::GetGlobalFunctionByIndex(asUINT uiIndex)
{
	auto& scriptModule = *GetScriptModuleFromScriptContext(asGetActiveContext());

	if (auto function = scriptModule.GetFunctionByIndex(uiIndex))
	{
		function->AddRef();
		return function;
	}

	return nullptr;
}

asITypeInfo* CASModuleReflectionGroup::FindTypeInfo(const std::string& name, bool searchByDecl)
{
	auto& scriptModule = *GetScriptModuleFromScriptContext(asGetActiveContext());

	const std::string oldNamespace = scriptModule.GetDefaultNamespace();

	asITypeInfo* type;

	if (searchByDecl)
	{
		const std::string currentNamespace = asutils::ExtractNamespaceFromDecl(name, false);

		scriptModule.SetDefaultNamespace(currentNamespace.c_str());

		type = scriptModule.GetTypeInfoByDecl(name.c_str());
	}
	else
	{
		const std::string currentNamespace = asutils::ExtractNamespaceFromName(name);
		const std::string actualName = asutils::ExtractNameFromName(name);

		scriptModule.SetDefaultNamespace(currentNamespace.c_str());

		type = scriptModule.GetTypeInfoByName(actualName.c_str());
	}

	scriptModule.SetDefaultNamespace(oldNamespace.c_str());

	if (type)
	{
		type->AddRef();
	}

	return type;
}

asUINT CASModuleReflectionGroup::GetObjectTypeCount() const
{
	auto& scriptModule = *GetScriptModuleFromScriptContext(asGetActiveContext());

	return scriptModule.GetObjectTypeCount();
}

asITypeInfo* CASModuleReflectionGroup::GetObjectTypeByIndex(asUINT uiIndex)
{
	auto& scriptModule = *GetScriptModuleFromScriptContext(asGetActiveContext());

	if (auto type = scriptModule.GetObjectTypeByIndex(uiIndex))
	{
		type->AddRef();
		return type;
	}

	return nullptr;
}
}
