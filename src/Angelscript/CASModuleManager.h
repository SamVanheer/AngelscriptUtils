#ifndef ANGELSCRIPT_CASMODULEMANAGER_H
#define ANGELSCRIPT_CASMODULEMANAGER_H

#include <memory>
#include <unordered_map>
#include <vector>

#include <angelscript.h>

#include "util/StringUtils.h"

#include "CASModuleDescriptor.h"

class CASManager;
class CASModule;
class IASModuleBuilder;

class CASModuleManager final
{
private:
	typedef std::unordered_map<const char*, std::unique_ptr<CASModuleDescriptor>, Hash_C_String<const char*>, EqualTo_C_String<const char*>> Descriptors_t;
	typedef std::vector<CASModule*> Modules_t;

public:
	CASModuleManager( CASManager& manager );
	~CASModuleManager() = default;

	const CASModuleDescriptor* FindDescriptorByName( const char* const pszTypeName ) const;

	bool AddDescriptor( const char* const pszTypeName, const asDWORD accessMask, const as::ModulePriority_t priority = as::ModulePriority::NORMAL );

	CASModule* BuildModule( const CASModuleDescriptor& descriptor, const char* const pszModuleName, IASModuleBuilder& builder );

	CASModule* BuildModule( const char* const pszTypeName, const char* const pszModuleName, IASModuleBuilder& builder );

private:
	CASModule* BuildModuleInternal( const CASModuleDescriptor& descriptor, const char* const pszModuleName, IASModuleBuilder& builder );

public:
	size_t GetModuleCount() const;

	const CASModule* FindModuleByName( const char* const pszModuleName ) const;

	CASModule* FindModuleByName( const char* const pszModuleName );

	const CASModule* FindModuleByIndex( const size_t uiIndex ) const;

	CASModule* FindModuleByIndex( const size_t uiIndex );

	bool AddModule( CASModule* pModule );

	void RemoveModule( CASModule* pModule );

	void RemoveModule( const char* const pszModuleName );

	/**
	*	Removes all modules and descriptors.
	*/
	void Clear();

private:
	CASManager& m_Manager;

	Descriptors_t m_Descriptors;

	as::DescriptorID_t m_NextDescriptorID = as::FIRST_DESCRIPTOR_ID;

	Modules_t m_Modules;

private:
	CASModuleManager( const CASModuleManager& ) = delete;
	CASModuleManager& operator=( const CASModuleManager& ) = delete;
};

#endif //ANGELSCRIPT_CASMODULEMANAGER_H