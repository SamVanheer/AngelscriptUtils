#include "AngelscriptUtils/CASModule.h"

#include "AngelscriptUtils/ScriptAPI/Reflection/CASModuleReflectionGroup.h"
#include "AngelscriptUtils/ScriptAPI/Reflection/ReflectionGroup.h"
#include "AngelscriptUtils/ScriptAPI/Reflection/TypeProviders.h"

namespace asutils
{
asIScriptFunction* CASModuleReflectionGroup::FindGlobalFunction(const std::string& name, bool searchByDecl)
{
	ModuleTypeProvider provider{*GetScriptModuleFromScriptContext(asGetActiveContext())};

	return ReflectionGroup::FindFunction(provider, name, searchByDecl);
}

asUINT CASModuleReflectionGroup::GetGlobalFunctionCount() const
{
	ModuleTypeProvider provider{*GetScriptModuleFromScriptContext(asGetActiveContext())};

	return ReflectionGroup::GetFunctionCount(provider);
}

asIScriptFunction* CASModuleReflectionGroup::GetGlobalFunctionByIndex(asUINT index)
{
	ModuleTypeProvider provider{*GetScriptModuleFromScriptContext(asGetActiveContext())};

	return ReflectionGroup::GetFunctionByIndex(provider, index);
}

asITypeInfo* CASModuleReflectionGroup::FindTypeInfo(const std::string& name, bool searchByDecl)
{
	ModuleTypeProvider provider{*GetScriptModuleFromScriptContext(asGetActiveContext())};

	return ReflectionGroup::FindTypeInfo(provider, name, searchByDecl);
}

asUINT CASModuleReflectionGroup::GetObjectTypeCount() const
{
	ModuleTypeProvider provider{*GetScriptModuleFromScriptContext(asGetActiveContext())};

	return ReflectionGroup::GetObjectTypeCount(provider);
}

asITypeInfo* CASModuleReflectionGroup::GetObjectTypeByIndex(asUINT index)
{
	ModuleTypeProvider provider{*GetScriptModuleFromScriptContext(asGetActiveContext())};

	return ReflectionGroup::GetObjectTypeByIndex(provider, index);
}
}
