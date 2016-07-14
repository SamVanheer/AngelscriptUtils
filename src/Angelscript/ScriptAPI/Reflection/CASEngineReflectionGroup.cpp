#include <cassert>

#include "CASEngineReflectionGroup.h"

asIScriptFunction* CASEngineReflectionGroup::FindGlobalFunction( const std::string& szName, bool bSearchByDecl )
{
	assert( m_pEngine );

	asIScriptFunction* pFunction = nullptr;

	if( bSearchByDecl )
	{
		pFunction = m_pEngine->GetGlobalFunctionByDecl( szName.c_str() );
	}
	else
	{
		const asUINT uiCount = m_pEngine->GetGlobalFunctionCount();

		for( asUINT uiIndex = 0; uiIndex < uiCount; ++uiIndex )
		{
			auto pFunc = m_pEngine->GetGlobalFunctionByIndex( uiIndex );

			//TODO: namespaces
			if( szName == pFunc->GetName() )
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

asUINT CASEngineReflectionGroup::GetGlobalFunctionCount() const
{
	assert( m_pEngine );

	return m_pEngine->GetGlobalFunctionCount();
}

asIScriptFunction* CASEngineReflectionGroup::GetGlobalFunctionByIndex( asUINT uiIndex )
{
	assert( m_pEngine );

	if( auto pFunction = m_pEngine->GetGlobalFunctionByIndex( uiIndex ) )
	{
		pFunction->AddRef();
		return pFunction;
	}

	return nullptr;
}

asITypeInfo* CASEngineReflectionGroup::FindTypeInfo( const std::string& szName, bool bSearchByDecl )
{
	assert( m_pEngine );

	//TODO: namespaces
	auto pType = bSearchByDecl ?
		m_pEngine->GetTypeInfoByDecl( szName.c_str() ) : 
		m_pEngine->GetTypeInfoByName( szName.c_str() );

	if( pType )
		pType->AddRef();

	return pType;
}

asUINT CASEngineReflectionGroup::GetObjectTypeCount() const
{
	assert( m_pEngine );

	return m_pEngine->GetObjectTypeCount();
}

asITypeInfo* CASEngineReflectionGroup::GetObjectTypeByIndex( asUINT uiIndex )
{
	assert( m_pEngine );

	if( auto pType = m_pEngine->GetObjectTypeByIndex( uiIndex ) )
	{
		pType->AddRef();
		return pType;
	}

	return nullptr;
}