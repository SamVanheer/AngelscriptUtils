#pragma once

#include <angelscript.h>

#include "CBaseEntity.h"

/**
*	Class name for CBaseEntity in scripts.
*/
#define AS_CBASEENTITY_CLASSNAME "CBaseEntity"

/**
*	Registers CBaseEntity for use in scripts.
*	@param engine Script engine.
*/
inline void RegisterScriptCBaseEntity(asIScriptEngine& engine)
{
	const char* const pszObjectName = AS_CBASEENTITY_CLASSNAME;

	engine.RegisterObjectType(pszObjectName, 0, asOBJ_REF | asOBJ_NOCOUNT);

	engine.RegisterObjectMethod(pszObjectName, "void Spawn()", asMETHOD(CBaseEntity, Spawn), asCALL_THISCALL);
}

/**
*	These functions are used to call base class versions of methods.
*/

template<typename BASECLASS>
void BaseEntity_Spawn(BASECLASS* pThis)
{
	pThis->BASECLASS::Spawn();
}

template<typename BASECLASS>
int BaseEntity_ScheduleOfType(BASECLASS* pThis, const std::string& szName)
{
	return pThis->BASECLASS::ScheduleOfType(szName);
}

/**
*	Registers the base version of an entity's CBaseEntity methods. This allow scripts to call base class implementations.
*	@param engine Script engine.
*	@param pszObjectName Class name.
*	@tparam BASECLASS Entity class type.
*/
template<typename BASECLASS>
void RegisterScriptBaseEntity(asIScriptEngine& engine, const char* const pszObjectName)
{
	engine.RegisterObjectType(pszObjectName, 0, asOBJ_REF | asOBJ_NOCOUNT);

	engine.RegisterObjectMethod(pszObjectName, "void Spawn()", asFUNCTION(BaseEntity_Spawn<BASECLASS>), asCALL_CDECL_OBJFIRST);

	engine.RegisterObjectMethod(pszObjectName, "int ScheduleOfType(const string& in szName)", asFUNCTION(BaseEntity_ScheduleOfType<BASECLASS>), asCALL_CDECL_OBJFIRST);
}

/**
*	Registers the base version of CBaseEntity.
*	@param engine Script engine.
*/
inline void RegisterScriptBaseEntity(asIScriptEngine& engine)
{
	//The base name is just the regular name without the 'C'.
	RegisterScriptBaseEntity<CBaseEntity>(engine, AS_CBASEENTITY_CLASSNAME + 1);
}
