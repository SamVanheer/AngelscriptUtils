#include "AngelscriptUtils/utility/Objects.h"

namespace asutils
{
void* CreateObjectInstance(asIScriptEngine& engine, const asITypeInfo& type)
{
	return engine.CreateScriptObject(&type);
}

void ReleaseVarArg(asIScriptEngine& engine, void* pObject, const int iTypeId)
{
	if (!pObject)
		return;

	auto pTypeInfo = engine.GetTypeInfoById(iTypeId);

	//Handles are pointers to pointers, so dereference it.
	if (iTypeId & asTYPEID_OBJHANDLE)
	{
		pObject = *reinterpret_cast<void**>(pObject);
	}

	if (iTypeId & asTYPEID_MASK_OBJECT)
	{
		engine.ReleaseScriptObject(pObject, pTypeInfo);
	}
}

asIScriptFunction* TryGetFunctionFromVariableParameter(const asIScriptEngine& engine, void* object, const int typeId)
{
	if (!(typeId & asTYPEID_OBJHANDLE))
	{
		return nullptr;
	}

	auto type = engine.GetTypeInfoById(typeId);

	if (!(type->GetFlags() & asOBJ_FUNCDEF))
	{
		return nullptr;
	}

	return *reinterpret_cast<asIScriptFunction**>(object);
}
}
