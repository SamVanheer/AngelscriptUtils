#ifndef ANGELSCRIPT_CASARGUMENTS_H
#define ANGELSCRIPT_CASARGUMENTS_H

#include "util/CASBaseClass.h"

/*
* Everything related to handling arguments is in here
*/

/*
* Possible types that an argument can be
*/
enum ArgumentType_t
{
	AT_NONE			= 1 << 0,	//No type set
	AT_VOID			= 1 << 1,	//No return type
	AT_PRIMITIVE	= 1 << 2,
	AT_VALUE		= 1 << 3,
	AT_REF			= 1 << 4,
	AT_ENUM			= 1 << 5,	//It's an enum, treat as dword
};

/*
* Can store the value of an argument
*/
struct ArgumentValue_t
{
	union
	{
		asBYTE		byte;
		asWORD		word;
		asDWORD		dword;
		asQWORD		qword;

		float		flValue;
		double		dValue;

		void*		pValue;
	};

	ArgumentValue_t()
		: qword()
	{
	}

};

/*
* Represents a single argument
*/
class CASArgument
{
public:

	CASArgument()
		: m_iTypeId( - 1 )
		, m_ArgType( AT_NONE )
		, m_Value()
	{
	}

	~CASArgument();

	CASArgument( const CASArgument& other );
	CASArgument& operator=( const CASArgument& other );

	inline int GetTypeId() const { return m_iTypeId; }

	inline ArgumentType_t GetArgumentType() const { return m_ArgType; }

	inline ArgumentValue_t& GetArgumentValue() { return m_Value; }
	inline const ArgumentValue_t& GetArgumentValue() const { return m_Value; }

	bool HasValue() const { return !( m_ArgType & ( AT_VOID | AT_NONE ) ); }

	//For read access only!
	void* GetArgumentAsPointer() const;

	bool Set( int iTypeId, ArgumentType_t type, const ArgumentValue_t& value, bool fCopy = false );

	bool Set( const CASArgument& other );

	void Reset();

private:

	//Object type id
	int m_iTypeId;

	//Used to prevent looking up the type again if it involves expensive lookup
	ArgumentType_t m_ArgType;

	//The actual value.
	ArgumentValue_t m_Value;
};

/*
* This class can store a variable number of arguments
*/
class CASArguments : public CASRefCountedBaseClass
{
public:

	CASArguments();
	CASArguments( asIScriptGeneric* pArguments, size_t uiStartIndex = 0 );
	CASArguments( asIScriptFunction* pTargetFunc, va_list list );
	~CASArguments();

	void Release() const;

	CASArguments( const CASArguments& other );
	CASArguments& operator=( const CASArguments& other );

	CASArgument* GetArgumentList() const { return m_pArguments; }

	size_t GetCount() const { return m_uiCount; }

	bool HasArguments() const { return m_uiCount > 0; }

	void Assign( const CASArguments& other );

	void Clear();

	bool SetArguments( asIScriptGeneric* pArguments, size_t uiStartIndex = 0 );

	bool SetArguments( asIScriptFunction* pTargetFunc, va_list list );

private:
	CASArgument* m_pArguments;
	size_t m_uiCount;
};

#endif //ANGELSCRIPT_CASARGUMENTS_H
