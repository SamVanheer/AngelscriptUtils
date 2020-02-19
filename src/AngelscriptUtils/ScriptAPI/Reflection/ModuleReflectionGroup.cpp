#include "AngelscriptUtils/CASModule.h"

#include "AngelscriptUtils/ScriptAPI/Reflection/ModuleReflectionGroup.h"
#include "AngelscriptUtils/ScriptAPI/Reflection/ReflectionGroup.h"

#include "AngelscriptUtils/utility/TypeProviders.h"

namespace asutils
{
asIScriptFunction* ModuleReflectionGroup::FindGlobalFunction(const std::string& name, bool searchByDecl)
{
	ModuleTypeProvider provider{*GetScriptModuleFromScriptContext(asGetActiveContext())};

	return ReflectionGroup::FindFunction(provider, name, searchByDecl);
}

asUINT ModuleReflectionGroup::GetGlobalFunctionCount() const
{
	ModuleTypeProvider provider{*GetScriptModuleFromScriptContext(asGetActiveContext())};

	return ReflectionGroup::GetFunctionCount(provider);
}

asIScriptFunction* ModuleReflectionGroup::GetGlobalFunctionByIndex(asUINT index)
{
	ModuleTypeProvider provider{*GetScriptModuleFromScriptContext(asGetActiveContext())};

	return ReflectionGroup::GetFunctionByIndex(provider, index);
}

asITypeInfo* ModuleReflectionGroup::FindTypeInfo(const std::string& name, bool searchByDecl)
{
	ModuleTypeProvider provider{*GetScriptModuleFromScriptContext(asGetActiveContext())};

	return ReflectionGroup::FindTypeInfo(provider, name, searchByDecl);
}

asUINT ModuleReflectionGroup::GetObjectTypeCount() const
{
	ModuleTypeProvider provider{*GetScriptModuleFromScriptContext(asGetActiveContext())};

	return ReflectionGroup::GetObjectTypeCount(provider);
}

asITypeInfo* ModuleReflectionGroup::GetObjectTypeByIndex(asUINT index)
{
	ModuleTypeProvider provider{*GetScriptModuleFromScriptContext(asGetActiveContext())};

	return ReflectionGroup::GetObjectTypeByIndex(provider, index);
}
}
