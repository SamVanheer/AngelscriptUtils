#ifndef ANGELSCRIPT_CASMODULE_H
#define ANGELSCRIPT_CASMODULE_H

#include <cstring>

#include "util/CASBaseClass.h"

#include "CASModuleDescriptor.h"

class asIScriptModule;

#define CASMODULE_USER_DATA_ID 10001

class CASModule final : public CASAtomicRefCountedBaseClass
{
public:
	CASModule( asIScriptModule* pModule, const CASModuleDescriptor& descriptor );
	~CASModule();

	void Release() const;

	void Discard();

	asIScriptModule* GetModule() { return m_pModule; }

	const char* GetModuleName() const;

	const CASModuleDescriptor& GetDescriptor() const { return *m_pDescriptor; }

private:
	asIScriptModule* m_pModule;

	const CASModuleDescriptor* m_pDescriptor;

private:
	CASModule( const CASModule& ) = delete;
	CASModule& operator=( const CASModule& ) = delete;
};

CASModule* GetModuleFromScriptModule( const asIScriptModule* pModule );

CASModule* GetModuleFromScriptFunction( const asIScriptFunction* pFunction );

inline bool ModuleLess( const CASModule* pLHS, const CASModule* pRHS )
{
	if(  pLHS->GetDescriptor() < pRHS->GetDescriptor() )
		return true;

	if( pRHS->GetDescriptor() < pLHS->GetDescriptor() )
		return false;

	//Use the memory address for sorting. They need only be sorted in a unique order.
	return reinterpret_cast<intptr_t>( pLHS ) < reinterpret_cast<intptr_t>( pRHS );
}

struct ModuleEqualByName
{
	const char* const pszModuleName;

	ModuleEqualByName( const char* const pszModuleName )
		: pszModuleName( pszModuleName )
	{
	}

	bool operator()( const CASModule* pModule ) const
	{
		return strcmp( pModule->GetModuleName(), pszModuleName ) == 0;
	}
};

#endif //ANGELSCRIPT_CASMODULE_H