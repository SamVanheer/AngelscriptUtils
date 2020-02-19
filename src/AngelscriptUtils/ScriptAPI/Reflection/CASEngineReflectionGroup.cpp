#include <cassert>

#include "AngelscriptUtils/utility/TypeStringUtils.h"

#include "AngelscriptUtils/ScriptAPI/Reflection/CASEngineReflectionGroup.h"

namespace asutils
{
asIScriptFunction* CASEngineReflectionGroup::FindGlobalFunction(const std::string& name, bool searchByDecl)
{
	assert(m_pEngine);

	asIScriptFunction* function = nullptr;

	if (searchByDecl)
	{
		const std::string oldNamespace = m_pEngine->GetDefaultNamespace();

		const std::string currentNamespace = asutils::ExtractNamespaceFromDecl(name);

		m_pEngine->SetDefaultNamespace(currentNamespace.c_str());

		function = m_pEngine->GetGlobalFunctionByDecl(name.c_str());

		m_pEngine->SetDefaultNamespace(oldNamespace.c_str());
	}
	else
	{
		const std::string currentNamespace = asutils::ExtractNamespaceFromName(name);
		const std::string actualName = asutils::ExtractNameFromName(name);

		const asUINT count = m_pEngine->GetGlobalFunctionCount();

		for (asUINT index = 0; index < count; ++index)
		{
			auto candidateFunction = m_pEngine->GetGlobalFunctionByIndex(index);

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

asUINT CASEngineReflectionGroup::GetGlobalFunctionCount() const
{
	assert(m_pEngine);

	return m_pEngine->GetGlobalFunctionCount();
}

asIScriptFunction* CASEngineReflectionGroup::GetGlobalFunctionByIndex(asUINT index)
{
	assert(m_pEngine);

	if (auto function = m_pEngine->GetGlobalFunctionByIndex(index))
	{
		function->AddRef();
		return function;
	}

	return nullptr;
}

asITypeInfo* CASEngineReflectionGroup::FindTypeInfo(const std::string& name, bool searchByDecl)
{
	assert(m_pEngine);

	const std::string oldNamespace = m_pEngine->GetDefaultNamespace();

	asITypeInfo* type;

	if (searchByDecl)
	{
		const std::string currentNamespace = asutils::ExtractNamespaceFromDecl(name, false);

		m_pEngine->SetDefaultNamespace(currentNamespace.c_str());

		type = m_pEngine->GetTypeInfoByDecl(name.c_str());
	}
	else
	{
		const std::string currentNamespace = asutils::ExtractNamespaceFromName(name);
		const std::string actualName = asutils::ExtractNameFromName(name);

		m_pEngine->SetDefaultNamespace(currentNamespace.c_str());

		type = m_pEngine->GetTypeInfoByName(actualName.c_str());
	}

	m_pEngine->SetDefaultNamespace(oldNamespace.c_str());

	if (type)
	{
		type->AddRef();
	}

	return type;
}

asUINT CASEngineReflectionGroup::GetObjectTypeCount() const
{
	assert(m_pEngine);

	return m_pEngine->GetObjectTypeCount();
}

asITypeInfo* CASEngineReflectionGroup::GetObjectTypeByIndex(asUINT index)
{
	assert(m_pEngine);

	if (auto type = m_pEngine->GetObjectTypeByIndex(index))
	{
		type->AddRef();
		return type;
	}

	return nullptr;
}
}
