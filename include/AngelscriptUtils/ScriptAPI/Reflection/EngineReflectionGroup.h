#pragma once

#include "AngelscriptUtils/ScriptAPI/Reflection/IReflectionGroup.h"

namespace asutils
{
class EngineReflectionGroup : public IReflectionGroup
{
public:
	EngineReflectionGroup() = default;
	~EngineReflectionGroup() = default;

	asIScriptFunction* FindGlobalFunction(const std::string& name, bool searchByDecl = false) override;

	asUINT GetGlobalFunctionCount() const override;

	asIScriptFunction* GetGlobalFunctionByIndex(asUINT index) override;

	asITypeInfo* FindTypeInfo(const std::string& name, bool searchByDecl = false) override;

	asUINT GetObjectTypeCount() const override;

	asITypeInfo* GetObjectTypeByIndex(asUINT index) override;

private:
	EngineReflectionGroup(const EngineReflectionGroup&) = delete;
	EngineReflectionGroup& operator=(const EngineReflectionGroup&) = delete;
};
}
