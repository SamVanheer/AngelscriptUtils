#pragma once

#include <string>

#include <angelscript.h>

namespace asutils
{
class IReflectionGroup
{
public:
	virtual ~IReflectionGroup() = 0;

	virtual asIScriptFunction* FindGlobalFunction(const std::string& name, bool searchByDecl = false) = 0;

	virtual asUINT GetGlobalFunctionCount() const = 0;

	virtual asIScriptFunction* GetGlobalFunctionByIndex(asUINT index) = 0;

	virtual asITypeInfo* FindTypeInfo(const std::string& name, bool searchByDecl = false) = 0;

	virtual asUINT GetObjectTypeCount() const = 0;

	virtual asITypeInfo* GetObjectTypeByIndex(asUINT index) = 0;
};

inline IReflectionGroup::~IReflectionGroup()
{
}
}
