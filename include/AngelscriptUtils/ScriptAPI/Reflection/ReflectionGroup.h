#pragma once

namespace asutils
{
class ITypeProvider;

/**
*	@brief Provides the common logic for reflection groups
*/
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