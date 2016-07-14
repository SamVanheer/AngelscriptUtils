#include "Angelscript/CASModule.h"

#include "CASModuleReflectionGroup.h"

asIScriptFunction* CASModuleReflectionGroup::FindGlobalFunction( const std::string& szName, bool bSearchByDecl )
{
	auto& scriptModule = *GetScriptModuleFromScriptContext( asGetActiveContext() );

	asIScriptFunction* pFunction = nullptr;

	if( bSearchByDecl )
	{
		pFunction = scriptModule.GetFunctionByDecl( szName.c_str() );
	}
	else
	{
		//TODO: namespaces
		pFunction = scriptModule.GetFunctionByName( szName.c_str() );
	}

	if( pFunction )
		pFunction->AddRef();

	return pFunction;
}

asUINT CASModuleReflectionGroup::GetGlobalFunctionCount() const
{
	auto& scriptModule = *GetScriptModuleFromScriptContext( asGetActiveContext() );

	return scriptModule.GetFunctionCount();
}

asIScriptFunction* CASModuleReflectionGroup::GetGlobalFunctionByIndex( asUINT uiIndex )
{
	auto& scriptModule = *GetScriptModuleFromScriptContext( asGetActiveContext() );

	if( auto pFunction = scriptModule.GetFunctionByIndex( uiIndex ) )
	{
		pFunction->AddRef();
		return pFunction;
	}

	return nullptr;
}

asITypeInfo* CASModuleReflectionGroup::FindTypeInfo( const std::string& szName, bool bSearchByDecl )
{
	auto& scriptModule = *GetScriptModuleFromScriptContext( asGetActiveContext() );

	//TODO: namespaces
	auto pType = bSearchByDecl ?
		scriptModule.GetTypeInfoByDecl( szName.c_str() ) :
		scriptModule.GetTypeInfoByName( szName.c_str() );

	if( pType )
		pType->AddRef();

	return pType;
}

asUINT CASModuleReflectionGroup::GetObjectTypeCount() const
{
	auto& scriptModule = *GetScriptModuleFromScriptContext( asGetActiveContext() );

	return scriptModule.GetObjectTypeCount();
}

asITypeInfo* CASModuleReflectionGroup::GetObjectTypeByIndex( asUINT uiIndex )
{
	auto& scriptModule = *GetScriptModuleFromScriptContext( asGetActiveContext() );

	if( auto pType = scriptModule.GetObjectTypeByIndex( uiIndex ) )
	{
		pType->AddRef();
		return pType;
	}

	return nullptr;
}