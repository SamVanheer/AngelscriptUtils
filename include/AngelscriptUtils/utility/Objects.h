#pragma once

#include <string>

#include <angelscript.h>

namespace asutils
{
/**
*	@brief Creates an instance of an object using its default constructor
*	@return Object instance, or null if the object could not be instantiated
*/
void* CreateObjectInstance(asIScriptEngine& engine, const asITypeInfo& type);

/**
*	@brief Releases a vararg argument
*/
void ReleaseVarArg(asIScriptEngine& engine, void* pObject, const int typeId);

/**
*	@brief Sets a global variable by name
*	@param module Module that contains the global variable
*	@param globalName Name of the global variable to set
*	@param value Value to set
*	@return true on success, false otherwise
*/
template<typename T>
inline bool SetGlobalByName(asIScriptModule& module, const std::string globalName, T value)
{
	const int index = module.GetGlobalVarIndexByName(globalName.c_str());

	if (index >= 0)
	{
		void* global = module.GetAddressOfGlobalVar(index);

		*(reinterpret_cast<T*>(global)) = value;

		return true;
	}

	return false;
}

/**
*	@brief Sets a global variable by name
*	@param module Module that contains the global variable
*	@param declaration Declaration of the global variable to set
*	@param value Value to set
*	@return true on success, false otherwise
*/
template<typename T>
inline bool SetGlobalByDecl(asIScriptModule& module, const std::string& declaration, T value)
{
	const int index = module.GetGlobalVarIndexByDecl(declaration.c_str());

	if (index >= 0)
	{
		void* global = module.GetAddressOfGlobalVar(index);

		*(reinterpret_cast<T*>(global)) = value;

		return true;
	}

	return false;
}

/**
*	Sets a value on an any object, which is an instance of a class that has the interface defined by CScriptAny
*	Angelscript will select the int64 or double overload when any integer or float type is passed, so this is necessary
*	@param object Object to set in the any
*	@param typeId Type Id
*/
template<typename ANY>
void SetAny(ANY& any, void* object, int typeId)
{
	//Have to convert primitive types of the form signed int and float smaller than their largest so that any.retrieve works correctly.
	asINT64 integerValue = 0;
	double floatValue = 0.0;

	//TODO: do unsigned types work properly with any?

	switch (typeId)
	{
	case asTYPEID_INT8:
	case asTYPEID_UINT8:	integerValue = *static_cast<int8_t*>(object); break;
	case asTYPEID_INT16:
	case asTYPEID_UINT16:	integerValue = *static_cast<int16_t*>(object); break;
	case asTYPEID_INT32:
	case asTYPEID_UINT32:	integerValue = *static_cast<int32_t*>(object); break;

	case asTYPEID_FLOAT:	floatValue = *static_cast<float*>(object); break;

	default: break;
	}

	switch (typeId)
	{
	case asTYPEID_INT8:
	case asTYPEID_UINT8:
	case asTYPEID_INT16:
	case asTYPEID_UINT16:
	case asTYPEID_INT32:
	case asTYPEID_UINT32:
		object = &integerValue;
		iTypeId = asTYPEID_INT64;
		break;

	case asTYPEID_FLOAT:
		object = &floatValue;
		iTypeId = asTYPEID_DOUBLE;
		break;

	default: break;
	}

	any.Store(object, typeId);
}
}
