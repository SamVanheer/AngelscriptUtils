#pragma once

#include "AngelscriptUtils/ScriptAPI/Reflection/IASReflectionGroup.h"

namespace asutils
{
class CASModuleReflectionGroup : public IASReflectionGroup
{
public:
	CASModuleReflectionGroup() = default;
	~CASModuleReflectionGroup() = default;

	asIScriptFunction* FindGlobalFunction(const std::string& name, bool searchByDecl = false) override;

	asUINT GetGlobalFunctionCount() const override;

	asIScriptFunction* GetGlobalFunctionByIndex(asUINT uiIndex) override;

	asITypeInfo* FindTypeInfo(const std::string& name, bool searchByDecl = false) override;

	asUINT GetObjectTypeCount() const override;

	asITypeInfo* GetObjectTypeByIndex(asUINT uiIndex) override;

private:
	CASModuleReflectionGroup(const CASModuleReflectionGroup&) = delete;
	CASModuleReflectionGroup& operator=(const CASModuleReflectionGroup&) = delete;
};
}