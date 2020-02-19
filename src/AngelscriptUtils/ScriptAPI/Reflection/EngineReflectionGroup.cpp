#include "AngelscriptUtils/ScriptAPI/Reflection/EngineReflectionGroup.h"
#include "AngelscriptUtils/ScriptAPI/Reflection/ReflectionGroup.h"

#include "AngelscriptUtils/utility/TypeProviders.h"

namespace asutils
{
asIScriptFunction* EngineReflectionGroup::FindGlobalFunction(const std::string& name, bool searchByDecl)
{
	EngineTypeProvider provider{*asGetActiveContext()->GetEngine()};

	return ReflectionGroup::FindFunction(provider, name, searchByDecl);
}

asUINT EngineReflectionGroup::GetGlobalFunctionCount() const
{
	EngineTypeProvider provider{*asGetActiveContext()->GetEngine()};

	return ReflectionGroup::GetFunctionCount(provider);
}

asIScriptFunction* EngineReflectionGroup::GetGlobalFunctionByIndex(asUINT index)
{
	EngineTypeProvider provider{*asGetActiveContext()->GetEngine()};

	return ReflectionGroup::GetFunctionByIndex(provider, index);
}

asITypeInfo* EngineReflectionGroup::FindTypeInfo(const std::string& name, bool searchByDecl)
{
	EngineTypeProvider provider{*asGetActiveContext()->GetEngine()};

	return ReflectionGroup::FindTypeInfo(provider, name, searchByDecl);
}

asUINT EngineReflectionGroup::GetObjectTypeCount() const
{
	EngineTypeProvider provider{*asGetActiveContext()->GetEngine()};

	return ReflectionGroup::GetObjectTypeCount(provider);
}

asITypeInfo* EngineReflectionGroup::GetObjectTypeByIndex(asUINT index)
{
	EngineTypeProvider provider{*asGetActiveContext()->GetEngine()};

	return ReflectionGroup::GetObjectTypeByIndex(provider, index);
}
}
