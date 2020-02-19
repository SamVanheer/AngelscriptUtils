#include "AngelscriptUtils/ScriptAPI/Reflection/ReflectionGroup.h"

#include "AngelscriptUtils/utility/TypeProviders.h"
#include "AngelscriptUtils/utility/TypeStringUtils.h"

namespace asutils
{
asIScriptFunction* ReflectionGroup::FindFunction(ITypeProvider& provider, const std::string& name, bool searchByDecl)
{
	asIScriptFunction* function = nullptr;

	if (searchByDecl)
	{
		const std::string oldNamespace = provider.GetDefaultNamespace();

		const std::string currentNamespace = asutils::ExtractNamespaceFromDecl(name);

		provider.SetDefaultNamespace(currentNamespace.c_str());

		function = provider.GetFunctionByDecl(name.c_str());

		provider.SetDefaultNamespace(oldNamespace.c_str());
	}
	else
	{
		const std::string currentNamespace = asutils::ExtractNamespaceFromName(name);
		const std::string actualName = asutils::ExtractNameFromName(name);

		const asUINT count = provider.GetFunctionCount();

		for (asUINT index = 0; index < count; ++index)
		{
			auto candidateFunction = provider.GetFunctionByIndex(index);

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

asUINT ReflectionGroup::GetFunctionCount(const ITypeProvider& provider)
{
	return provider.GetFunctionCount();
}

asIScriptFunction* ReflectionGroup::GetFunctionByIndex(ITypeProvider& provider, asUINT uiIndex)
{
	if (auto function = provider.GetFunctionByIndex(uiIndex))
	{
		function->AddRef();
		return function;
	}

	return nullptr;
}

asITypeInfo* ReflectionGroup::FindTypeInfo(ITypeProvider& provider, const std::string& name, bool searchByDecl)
{
	const std::string oldNamespace = provider.GetDefaultNamespace();

	asITypeInfo* type;

	if (searchByDecl)
	{
		const std::string currentNamespace = asutils::ExtractNamespaceFromDecl(name, false);

		provider.SetDefaultNamespace(currentNamespace.c_str());

		type = provider.GetTypeInfoByDecl(name.c_str());
	}
	else
	{
		const std::string currentNamespace = asutils::ExtractNamespaceFromName(name);
		const std::string actualName = asutils::ExtractNameFromName(name);

		provider.SetDefaultNamespace(currentNamespace.c_str());

		type = provider.GetTypeInfoByName(actualName.c_str());
	}

	provider.SetDefaultNamespace(oldNamespace.c_str());

	if (type)
	{
		type->AddRef();
	}

	return type;
}

asUINT ReflectionGroup::GetObjectTypeCount(const ITypeProvider& provider)
{
	return provider.GetObjectTypeCount();
}

asITypeInfo* ReflectionGroup::GetObjectTypeByIndex(ITypeProvider& provider, asUINT uiIndex)
{
	if (auto type = provider.GetObjectTypeByIndex(uiIndex))
	{
		type->AddRef();
		return type;
	}

	return nullptr;
}
}
