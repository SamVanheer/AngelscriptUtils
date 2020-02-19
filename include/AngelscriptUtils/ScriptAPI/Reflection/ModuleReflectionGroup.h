#pragma once

#include "AngelscriptUtils/ScriptAPI/Reflection/IReflectionGroup.h"

namespace asutils
{
class ModuleReflectionGroup : public IReflectionGroup
{
public:
	ModuleReflectionGroup() = default;
	~ModuleReflectionGroup() = default;

	asIScriptFunction* FindGlobalFunction(const std::string& name, bool searchByDecl = false) override;

	asUINT GetGlobalFunctionCount() const override;

	asIScriptFunction* GetGlobalFunctionByIndex(asUINT uiIndex) override;

	asITypeInfo* FindTypeInfo(const std::string& name, bool searchByDecl = false) override;

	asUINT GetObjectTypeCount() const override;

	asITypeInfo* GetObjectTypeByIndex(asUINT uiIndex) override;

private:
	ModuleReflectionGroup(const ModuleReflectionGroup&) = delete;
	ModuleReflectionGroup& operator=(const ModuleReflectionGroup&) = delete;
};
}