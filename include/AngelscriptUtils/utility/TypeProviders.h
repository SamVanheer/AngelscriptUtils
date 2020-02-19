#pragma once

#include <type_traits>

#include <angelscript.h>

namespace asutils
{
/**
*	@brief Abstracts access to the list of types and global functions defined in the engine and in individual modules
*/
class ITypeProvider
{
public:
	virtual ~ITypeProvider() = 0;

	virtual const char* GetDefaultNamespace() const = 0;

	virtual int SetDefaultNamespace(const char* defaultNamespace) = 0;

	virtual asIScriptFunction* GetFunctionByDecl(const char* declaration) const = 0;

	virtual asUINT GetFunctionCount() const = 0;

	virtual asIScriptFunction* GetFunctionByIndex(asUINT index) const = 0;

	virtual asITypeInfo* GetTypeInfoByDecl(const char* declaration) const = 0;

	virtual asITypeInfo* GetTypeInfoByName(const char* name) const = 0;

	virtual asUINT GetObjectTypeCount() const = 0;

	virtual asITypeInfo* GetObjectTypeByIndex(asUINT index) const = 0;
};

class EngineTypeProvider final : public ITypeProvider
{
public:
	EngineTypeProvider(asIScriptEngine& engine)
		: m_Engine(engine)
	{
	}

	const char* GetDefaultNamespace() const override { return m_Engine.GetDefaultNamespace(); }

	int SetDefaultNamespace(const char* defaultNamespace) override { return m_Engine.SetDefaultNamespace(defaultNamespace); }

	asIScriptFunction* GetFunctionByDecl(const char* declaration) const override { return m_Engine.GetGlobalFunctionByDecl(declaration); }

	asUINT GetFunctionCount() const override { return m_Engine.GetGlobalFunctionCount(); }

	asIScriptFunction* GetFunctionByIndex(asUINT index) const override { return m_Engine.GetGlobalFunctionByIndex(index); }

	asITypeInfo* GetTypeInfoByDecl(const char* declaration) const override { return m_Engine.GetTypeInfoByDecl(declaration); }

	asITypeInfo* GetTypeInfoByName(const char* name) const override { return m_Engine.GetTypeInfoByName(name); };

	asUINT GetObjectTypeCount() const override { return m_Engine.GetObjectTypeCount(); }

	asITypeInfo* GetObjectTypeByIndex(asUINT index) const override { return m_Engine.GetObjectTypeByIndex(index); }

private:
	asIScriptEngine& m_Engine;
};

class ModuleTypeProvider final : public ITypeProvider
{
public:
	ModuleTypeProvider(asIScriptModule& module)
		: m_Module(module)
	{
	}

	const char* GetDefaultNamespace() const override { return m_Module.GetDefaultNamespace(); }

	int SetDefaultNamespace(const char* defaultNamespace) override { return m_Module.SetDefaultNamespace(defaultNamespace); }

	asIScriptFunction* GetFunctionByDecl(const char* declaration) const override { return m_Module.GetFunctionByDecl(declaration); }

	asUINT GetFunctionCount() const override { return m_Module.GetFunctionCount(); }

	asIScriptFunction* GetFunctionByIndex(asUINT index) const override { return m_Module.GetFunctionByIndex(index); }

	asITypeInfo* GetTypeInfoByDecl(const char* declaration) const override { return m_Module.GetTypeInfoByDecl(declaration); }

	asITypeInfo* GetTypeInfoByName(const char* name) const override { return m_Module.GetTypeInfoByName(name); };

	asUINT GetObjectTypeCount() const override { return m_Module.GetObjectTypeCount(); }

	asITypeInfo* GetObjectTypeByIndex(asUINT index) const override { return m_Module.GetObjectTypeByIndex(index); }

private:
	asIScriptModule& m_Module;
};
}
