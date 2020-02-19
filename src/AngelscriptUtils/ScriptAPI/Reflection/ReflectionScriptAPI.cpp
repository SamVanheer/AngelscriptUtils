#include "AngelscriptUtils/ScriptAPI/Reflection/EngineReflectionGroup.h"
#include "AngelscriptUtils/ScriptAPI/Reflection/IReflectionGroup.h"
#include "AngelscriptUtils/ScriptAPI/Reflection/ModuleReflectionGroup.h"
#include "AngelscriptUtils/ScriptAPI/Reflection/ReflectionScriptAPI.h"

namespace asutils
{
namespace Reflect
{
static EngineReflectionGroup Engine;
static ModuleReflectionGroup Module;
}

static void Function_AddRef(const asIScriptFunction* const instance)
{
	instance->AddRef();
}

static void Function_Release(const asIScriptFunction* const instance)
{
	instance->Release();
}

static asITypeInfo* Function_GetObjectType(const asIScriptFunction* const instance)
{
	if (auto type = instance->GetObjectType())
	{
		instance->AddRef();
		return type;
	}

	return nullptr;
}

static std::string Function_GetObjectName(const asIScriptFunction* const instance)
{
	if (auto name = instance->GetObjectName())
	{
		return name;
	}

	return "";
}

static std::string Function_GetNamespace(const asIScriptFunction* const instance)
{
	if (auto functionNamespace = instance->GetNamespace())
	{
		return functionNamespace;
	}

	return "";
}

static std::string Function_GetName(const asIScriptFunction* const instance)
{
	return instance->GetName();
}

static std::string Function_GetDeclaration(const asIScriptFunction* const instance,
	const bool includeObjectName = true, const bool includeNamespace = false, const bool includeParamNames = false)
{
	return instance->GetDeclaration(includeObjectName, includeNamespace, includeParamNames);
}

static void RegisterScriptReflectionCallable(asIScriptEngine& engine, const char* const pszObjectName)
{
	engine.RegisterObjectType(pszObjectName, 0, asOBJ_REF);

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_ADDREF, "void AddRef()",
		asFUNCTION(Function_AddRef), asCALL_CDECL_OBJFIRST);

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_RELEASE, "void Release()",
		asFUNCTION(Function_Release), asCALL_CDECL_OBJFIRST);

	engine.RegisterObjectMethod(
		pszObjectName, "TypeInfo@ GetObjectType() const",
		asFUNCTION(Function_GetObjectType), asCALL_CDECL_OBJFIRST);

	engine.RegisterObjectMethod(
		pszObjectName, "string GetObjectName() const",
		asFUNCTION(Function_GetObjectName), asCALL_CDECL_OBJFIRST);

	engine.RegisterObjectMethod(
		pszObjectName, "string GetNamespace() const",
		asFUNCTION(Function_GetNamespace), asCALL_CDECL_OBJFIRST);

	engine.RegisterObjectMethod(
		pszObjectName, "string GetName() const",
		asFUNCTION(Function_GetName), asCALL_CDECL_OBJFIRST);

	engine.RegisterObjectMethod(
		pszObjectName, "string GetDeclaration(const bool includeObjectName = true, const bool includeNamespace = false, const bool includeParamNames = false) const",
		asFUNCTION(Function_GetDeclaration), asCALL_CDECL_OBJFIRST);

	engine.RegisterObjectMethod(
		pszObjectName, "bool IsReadOnly() const",
		asMETHOD(asIScriptFunction, IsReadOnly), asCALL_THISCALL);

	engine.RegisterObjectMethod(
		pszObjectName, "bool IsPrivate() const",
		asMETHOD(asIScriptFunction, IsPrivate), asCALL_THISCALL);

	engine.RegisterObjectMethod(
		pszObjectName, "bool IsProtected() const",
		asMETHOD(asIScriptFunction, IsProtected), asCALL_THISCALL);

	engine.RegisterObjectMethod(
		pszObjectName, "bool IsFinal() const",
		asMETHOD(asIScriptFunction, IsFinal), asCALL_THISCALL);

	engine.RegisterObjectMethod(
		pszObjectName, "bool IsOverride() const",
		asMETHOD(asIScriptFunction, IsOverride), asCALL_THISCALL);

	engine.RegisterObjectMethod(
		pszObjectName, "bool IsShared() const",
		asMETHOD(asIScriptFunction, IsShared), asCALL_THISCALL);
}

static void RegisterScriptReflectionFunction(asIScriptEngine& engine)
{
	RegisterScriptReflectionCallable(engine, "Function");
}

static void RegisterScriptReflectionMethod(asIScriptEngine& engine)
{
	RegisterScriptReflectionCallable(engine, "Method");
}

static void TypeInfo_AddRef(const asITypeInfo* const instance)
{
	instance->AddRef();
}

static void TypeInfo_Release(const asITypeInfo* const instance)
{
	instance->Release();
}

static std::string TypeInfo_GetNamespace(const asITypeInfo* const instance)
{
	if (auto functionNamespace = instance->GetNamespace())
	{
		return functionNamespace;
	}

	return "";
}

static std::string TypeInfo_GetName(const asITypeInfo* const instance)
{
	return instance->GetName();
}

static asIScriptFunction* TypeInfo_GetMethodByIndex(const asITypeInfo* const instance, const asUINT index, const bool getVirtual)
{
	if (auto function = instance->GetMethodByIndex(index, getVirtual))
	{
		function->AddRef();
		return function;
	}

	return nullptr;
}

static asIScriptFunction* TypeInfo_GetMethodByDecl(const asITypeInfo* const instance, const std::string& declaration, const bool getVirtual)
{
	if (auto function = instance->GetMethodByDecl(declaration.c_str(), getVirtual))
	{
		function->AddRef();
		return function;
	}

	return nullptr;
}

static asIScriptFunction* TypeInfo_GetMethodByName(const asITypeInfo* const instance, const std::string& name, const bool getVirtual)
{
	if (auto function = instance->GetMethodByName(name.c_str(), getVirtual))
	{
		function->AddRef();
		return function;
	}

	return nullptr;
}

static void RegisterScriptReflectionTypeInfo(asIScriptEngine& engine)
{
	const char* const pszObjectName = "TypeInfo";

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_ADDREF, "void AddRef()",
		asFUNCTION(TypeInfo_AddRef), asCALL_CDECL_OBJFIRST);

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_RELEASE, "void Release()",
		asFUNCTION(TypeInfo_Release), asCALL_CDECL_OBJFIRST);

	engine.RegisterObjectMethod(
		pszObjectName, "string GetNamespace() const",
		asFUNCTION(TypeInfo_GetNamespace), asCALL_CDECL_OBJFIRST);

	engine.RegisterObjectMethod(
		pszObjectName, "string GetName() const",
		asFUNCTION(TypeInfo_GetName), asCALL_CDECL_OBJFIRST);

	engine.RegisterObjectMethod(
		pszObjectName, "uint GetMethodCount() const",
		asMETHOD(asITypeInfo, GetMethodCount), asCALL_THISCALL);

	engine.RegisterObjectMethod(
		pszObjectName, "Method@ GetMethodByIndex(const uint index, const bool getVirtual = true) const",
		asFUNCTION(TypeInfo_GetMethodByIndex), asCALL_CDECL_OBJFIRST);

	engine.RegisterObjectMethod(
		pszObjectName, "Method@ GetMethodByDecl(const string& in declaration, const bool getVirtual = true) const",
		asFUNCTION(TypeInfo_GetMethodByDecl), asCALL_CDECL_OBJFIRST);

	engine.RegisterObjectMethod(
		pszObjectName, "Method@ GetMethodByName(const string& in name, const bool getVirtual = true) const",
		asFUNCTION(TypeInfo_GetMethodByName), asCALL_CDECL_OBJFIRST);
}

static void RegisterScriptReflectionGroup(asIScriptEngine& engine)
{
	const char* const pszObjectName = "IGroup";

	engine.RegisterObjectType(pszObjectName, 0, asOBJ_REF | asOBJ_NOCOUNT);

	engine.RegisterObjectMethod(
		pszObjectName, "Function@ FindGlobalFunction(const string& in name, bool searchByDecl = false)",
		asMETHOD(IReflectionGroup, FindGlobalFunction), asCALL_THISCALL);

	engine.RegisterObjectMethod(
		pszObjectName, "uint GetGlobalFunctionCount() const",
		asMETHOD(IReflectionGroup, GetGlobalFunctionCount), asCALL_THISCALL);

	engine.RegisterObjectMethod(
		pszObjectName, "Function@ GetGlobalFunctionByIndex(uint index)",
		asMETHOD(IReflectionGroup, GetGlobalFunctionByIndex), asCALL_THISCALL);

	engine.RegisterObjectMethod(
		pszObjectName, "TypeInfo@ FindTypeInfo(const string& in name, bool searchByDecl = false)",
		asMETHOD(IReflectionGroup, FindTypeInfo), asCALL_THISCALL);

	engine.RegisterObjectMethod(
		pszObjectName, "uint GetObjectTypeCount() const",
		asMETHOD(IReflectionGroup, GetObjectTypeCount), asCALL_THISCALL);

	engine.RegisterObjectMethod(
		pszObjectName, "TypeInfo@ GetObjectTypeByIndex(uint index) const",
		asMETHOD(IReflectionGroup, GetObjectTypeByIndex), asCALL_THISCALL);
}

void RegisterReflectionAPI(asIScriptEngine& engine)
{
	const std::string oldNamespace = engine.GetDefaultNamespace();

	engine.SetDefaultNamespace("Reflect");

	//Forward declarations.
	engine.RegisterObjectType("TypeInfo", 0, asOBJ_REF);

	RegisterScriptReflectionFunction(engine);
	RegisterScriptReflectionMethod(engine);
	RegisterScriptReflectionTypeInfo(engine);
	RegisterScriptReflectionGroup(engine);

	engine.RegisterGlobalProperty("IGroup Engine", &Reflect::Engine);
	engine.RegisterGlobalProperty("IGroup Module", &Reflect::Module);

	engine.SetDefaultNamespace(oldNamespace.c_str());
}
}
