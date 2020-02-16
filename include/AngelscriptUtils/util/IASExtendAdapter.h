#ifndef ANGELSCRIPT_UTIL_IASEXTENDADAPTER_H
#define ANGELSCRIPT_UTIL_IASEXTENDADAPTER_H

#include "AngelscriptUtils/utility/SmartPointers.h"

/**
*	@addtogroup ASExtend
*
*	@{
*/

/**
*	Interface used to adapt C++ classes for extension in Angelscript.
*/
class IASExtendAdapter
{
public:
	virtual ~IASExtendAdapter() = 0;

	/**
	*	@return The object pointer.
	*/
	virtual const asutils::ObjectPointer& GetObject() const = 0;

	/**
	*	@copydoc GetObject() const
	*/
	virtual asutils::ObjectPointer GetObject() = 0;
};

inline IASExtendAdapter::~IASExtendAdapter()
{
}

/** @} */

#endif //ANGELSCRIPT_UTIL_IASEXTENDADAPTER_H