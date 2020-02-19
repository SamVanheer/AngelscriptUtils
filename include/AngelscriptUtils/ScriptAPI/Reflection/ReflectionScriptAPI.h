#pragma once

class asIScriptEngine;

namespace asutils
{
/**
*	@brief Registers the Reflection API in the Reflect namespace, registering these types:
*	TypeInfo
*	Function
*	Method
*	IGroup
*	The following globals are registered:
*	Engine
*	Module
*	These globals are stateless and will operate on the engine or module currently calling the API
*/
void RegisterReflectionAPI(asIScriptEngine& engine);
}
