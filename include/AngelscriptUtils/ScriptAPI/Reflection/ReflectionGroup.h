#pragma once

#include "AngelscriptUtils/ScriptAPI/Reflection/IASReflectionGroup.h"

namespace asutils
{
class ITypeProvider;

class ReflectionGroup final
{
public:
	ReflectionGroup() = delete;

	static asIScriptFunction* FindFunction(ITypeProvider& provider, const std::string& name, bool searchByDecl = false);

	static asUINT GetFunctionCount(const ITypeProvider& provider);

	static asIScriptFunction* GetFunctionByIndex(ITypeProvider& provider, asUINT uiIndex);

	static asITypeInfo* FindTypeInfo(ITypeProvider& provider, const std::string& name, bool searchByDecl = false);

	static asUINT GetObjectTypeCount(const ITypeProvider& provider);

	static asITypeInfo* GetObjectTypeByIndex(ITypeProvider& provider, asUINT uiIndex);
};
}