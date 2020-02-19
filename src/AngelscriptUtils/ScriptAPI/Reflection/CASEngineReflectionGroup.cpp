#include "AngelscriptUtils/ScriptAPI/Reflection/CASEngineReflectionGroup.h"
#include "AngelscriptUtils/ScriptAPI/Reflection/ReflectionGroup.h"
#include "AngelscriptUtils/ScriptAPI/Reflection/TypeProviders.h"

namespace asutils
{
asIScriptFunction* CASEngineReflectionGroup::FindGlobalFunction(const std::string& name, bool searchByDecl)
{
	EngineTypeProvider provider{*asGetActiveContext()->GetEngine()};

	return ReflectionGroup::FindFunction(provider, name, searchByDecl);
}

asUINT CASEngineReflectionGroup::GetGlobalFunctionCount() const
{
	EngineTypeProvider provider{*asGetActiveContext()->GetEngine()};

	return ReflectionGroup::GetFunctionCount(provider);
}

asIScriptFunction* CASEngineReflectionGroup::GetGlobalFunctionByIndex(asUINT index)
{
	EngineTypeProvider provider{*asGetActiveContext()->GetEngine()};

	return ReflectionGroup::GetFunctionByIndex(provider, index);
}

asITypeInfo* CASEngineReflectionGroup::FindTypeInfo(const std::string& name, bool searchByDecl)
{
	EngineTypeProvider provider{*asGetActiveContext()->GetEngine()};

	return ReflectionGroup::FindTypeInfo(provider, name, searchByDecl);
}

asUINT CASEngineReflectionGroup::GetObjectTypeCount() const
{
	EngineTypeProvider provider{*asGetActiveContext()->GetEngine()};

	return ReflectionGroup::GetObjectTypeCount(provider);
}

asITypeInfo* CASEngineReflectionGroup::GetObjectTypeByIndex(asUINT index)
{
	EngineTypeProvider provider{*asGetActiveContext()->GetEngine()};

	return ReflectionGroup::GetObjectTypeByIndex(provider, index);
}
}
