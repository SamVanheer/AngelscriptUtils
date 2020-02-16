#include "AngelscriptUtils/CASModule.h"

#include "AngelscriptUtils/utility/TypeStringUtils.h"

#include "AngelscriptUtils/ScriptAPI/Reflection/CASModuleReflectionGroup.h"

asIScriptFunction* CASModuleReflectionGroup::FindGlobalFunction( const std::string& szName, bool bSearchByDecl )
{
	auto& scriptModule = *GetScriptModuleFromScriptContext( asGetActiveContext() );

	asIScriptFunction* pFunction = nullptr;

	if( bSearchByDecl )
	{
		const std::string szOldNS = scriptModule.GetDefaultNamespace();

		const std::string szNS = asutils::ExtractNamespaceFromDecl( szName );

		scriptModule.SetDefaultNamespace( szNS.c_str() );

		pFunction = scriptModule.GetFunctionByDecl( szName.c_str() );

		scriptModule.SetDefaultNamespace( szOldNS.c_str() );

		pFunction = scriptModule.GetFunctionByDecl( szName.c_str() );
	}
	else
	{
		pFunction = scriptModule.GetFunctionByName( szName.c_str() );

		const std::string szNS = asutils::ExtractNamespaceFromName( szName );
		const std::string szActualName = asutils::ExtractNameFromName( szName );

		const asUINT uiCount = scriptModule.GetFunctionCount();

		for( asUINT uiIndex = 0; uiIndex < uiCount; ++uiIndex )
		{
			auto pFunc = scriptModule.GetFunctionByIndex( uiIndex );

			if( szActualName == pFunc->GetName() && szNS == pFunc->GetNamespace() )
			{
				pFunction = pFunc;
				break;
			}
		}
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

	const std::string szOldNS = scriptModule.GetDefaultNamespace();

	asITypeInfo* pType;

	if( bSearchByDecl )
	{
		const std::string szNS = asutils::ExtractNamespaceFromDecl( szName, false );

		scriptModule.SetDefaultNamespace( szNS.c_str() );

		pType = scriptModule.GetTypeInfoByDecl( szName.c_str() );
	}
	else
	{
		const std::string szNS = asutils::ExtractNamespaceFromName( szName );
		const std::string szActualName = asutils::ExtractNameFromName( szName );

		scriptModule.SetDefaultNamespace( szNS.c_str() );

		pType = scriptModule.GetTypeInfoByName( szActualName.c_str() );
	}

	scriptModule.SetDefaultNamespace( szOldNS.c_str() );

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