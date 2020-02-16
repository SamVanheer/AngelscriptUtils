#include <cstdint>
#include <memory>
#include <sstream>

#include "AngelscriptUtils/util/ASUtil.h"
#include "AngelscriptUtils/utility/TypeInfo.h"

namespace as
{
void ReleaseVarArg( asIScriptEngine& engine, void* pObject, const int iTypeId )
{
	if( !pObject )
		return;

	auto pTypeInfo = engine.GetTypeInfoById( iTypeId );

	//Handles are pointers to pointers, so dereference it.
	if( iTypeId & asTYPEID_OBJHANDLE )
	{
		pObject = *reinterpret_cast<void**>( pObject );
	}

	if( iTypeId & asTYPEID_MASK_OBJECT )
	{
		engine.ReleaseScriptObject( pObject, pTypeInfo );
	}
}

void* CreateObjectInstance( asIScriptEngine& engine, const asITypeInfo& type )
{
	if( !asutils::HasDefaultConstructor( type ) )
		return nullptr;

	return engine.CreateScriptObject( &type );
}
}