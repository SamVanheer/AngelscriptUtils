#pragma once

#include "AngelscriptUtils/ScriptAPI/Reflection/IASReflectionGroup.h"

namespace asutils
{
class CASEngineReflectionGroup : public IASReflectionGroup
{
public:
	CASEngineReflectionGroup() = default;
	~CASEngineReflectionGroup() = default;

	asIScriptFunction* FindGlobalFunction(const std::string& name, bool searchByDecl = false) override;

	asUINT GetGlobalFunctionCount() const override;

	asIScriptFunction* GetGlobalFunctionByIndex(asUINT index) override;

	asITypeInfo* FindTypeInfo(const std::string& name, bool searchByDecl = false) override;

	asUINT GetObjectTypeCount() const override;

	asITypeInfo* GetObjectTypeByIndex(asUINT index) override;

	void SetEngine(asIScriptEngine& engine)
	{
		m_pEngine = &engine;
	}

private:
	asIScriptEngine* m_pEngine = nullptr;

private:
	CASEngineReflectionGroup(const CASEngineReflectionGroup&) = delete;
	CASEngineReflectionGroup& operator=(const CASEngineReflectionGroup&) = delete;
};
}
