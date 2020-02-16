#include "AngelscriptUtils/utility/TypeInfo.h"

namespace asutils
{
bool HasDefaultConstructor(const asITypeInfo& type)
{
	//Non-object types return 0 factories here.
	for (asUINT uiIndex = 0; uiIndex < type.GetFactoryCount(); ++uiIndex)
	{
		asIScriptFunction* pFactory = type.GetFactoryByIndex(uiIndex);

		//A default constructor has 0 parameters
		if (pFactory->GetParamCount() == 0)
			return true;
	}

	return false;
}
}
