#ifndef WRAPPER_CASCALLABLE_H
#define WRAPPER_CASCALLABLE_H

#include <cstdarg>
#include <cstdint>

#include <angelscript.h>

class CASContext;

typedef uint32_t CallFlags_t;

namespace CallFlag
{
enum CallFlag : CallFlags_t
{
	NONE = 0,
};
}

class CASCallable
{
protected:
	template<typename CALLABLE>
	friend bool CallFunction( CALLABLE& callable, CallFlags_t flags, va_list list );

protected:
	CASCallable( asIScriptFunction& function, CASContext& context );

public:
	asIScriptFunction& GetFunction() { return m_Function; }

	CASContext* GetContext() { return m_pContext; }

	bool IsValid() const;

protected:
	bool PreSetArguments() { return true; }
	bool PreExecute() { return true; }

	bool PostExecute( const int iResult ) { return true; }

private:
	asIScriptFunction& m_Function;
	CASContext* m_pContext;

private:
	CASCallable( const CASCallable& ) = delete;
	CASCallable& operator=( const CASCallable& ) = delete;
};

class CASFunction : public CASCallable
{
public:
	CASFunction( asIScriptFunction& function, CASContext& context )
		: CASCallable( function, context )
	{
	}

	bool VCall( CallFlags_t flags, va_list list );

	bool operator()( CallFlags_t flags, ... );
};

class CASMethod : public CASCallable
{
protected:
	template<typename CALLABLE>
	friend bool CallFunction( CALLABLE& callable, CallFlags_t flags, va_list list );

public:
	CASMethod( asIScriptFunction& function, CASContext& context, void* pThis )
		: CASCallable( function, context )
		, m_pThis( pThis )
	{
	}

	bool IsValid() const;

	bool VCall( CallFlags_t flags, va_list list );

	bool operator()( CallFlags_t flags, ... );

protected:
	bool PreSetArguments();

private:
	void* m_pThis;
};

namespace as
{
/**
*	Calls the given function using the given context.
*	@param pFunction Function to call.
*	@param pContext Context to use.
*	@param flags Flags.
*	@param list Argument list.
*/
bool VCallFunction( asIScriptFunction* pFunction, asIScriptContext* pContext, CallFlags_t flags, va_list list );

/**
*	@see VCallFunction( asIScriptFunction& function, asIScriptContext& context, CallFlags_t flags, va_list list )
*/
bool VCallFunction( asIScriptFunction* pFunction, asIScriptContext* pContext, va_list list );

/**
*	Acquires a context using asIScriptEngine::RequestContext
*	@see VCallFunction( asIScriptFunction& function, asIScriptContext& context, CallFlags_t flags, va_list list )
*/
bool VCallFunction( asIScriptFunction* pFunction, CallFlags_t flags, va_list list );

/**
*	Acquires a context using asIScriptEngine::RequestContext
*	@see VCallFunction( asIScriptFunction& function, asIScriptContext& context, CallFlags_t flags, va_list list )
*/
bool VCallFunction( asIScriptFunction* pFunction, va_list list );

/**
*	Calls the given function using the given context.
*	@param pFunction Function to call.
*	@param pContext Context to use.
*	@param flags Flags.
*	@param ... Arguments.
*/
bool CallFunction( asIScriptFunction* pFunction, asIScriptContext* pContext, CallFlags_t flags, ... );

/**
*	@see CallFunction( asIScriptFunction& function, asIScriptContext& context, CallFlags_t flags, ... )
*/
bool CallFunction( asIScriptFunction* pFunction, asIScriptContext* pContext, ... );

/**
*	Acquires a context using asIScriptEngine::RequestContext
*	@see CallFunction( asIScriptFunction& function, asIScriptContext& context, CallFlags_t flags, ... )
*/
bool CallFunction( asIScriptFunction* pFunction, CallFlags_t flags, ... );

/**
*	Acquires a context using asIScriptEngine::RequestContext
*	@see CallFunction( asIScriptFunction& function, asIScriptContext& context, CallFlags_t flags, ... )
*/
bool CallFunction( asIScriptFunction* pFunction, ... );

/**
*	Calls the given function using the given context.
*	@param pThis This pointer.
*	@param pFunction Function to call.
*	@param pContext Context to use.
*	@param flags Flags.
*	@param list Argument list.
*/
bool VCallMethod( void* pThis, asIScriptFunction* pFunction, asIScriptContext* pContext, CallFlags_t flags, va_list list );

/**
*	@see VCallMethod( void* pThis, asIScriptFunction& function, asIScriptContext& context, CallFlags_t flags, va_list list )
*/
bool VCallMethod( void* pThis, asIScriptFunction* pFunction, asIScriptContext* pContext, va_list list );

/**
*	Acquires a context using asIScriptEngine::RequestContext
*	@see VCallMethod( void* pThis, asIScriptFunction& function, asIScriptContext& context, CallFlags_t flags, va_list list )
*/
bool VCallMethod( void* pThis, asIScriptFunction* pFunction, CallFlags_t flags, va_list list );

/**
*	Acquires a context using asIScriptEngine::RequestContext
*	@see VCallMethod( void* pThis, asIScriptFunction& function, asIScriptContext& context, CallFlags_t flags, va_list list )
*/
bool VCallMethod( void* pThis, asIScriptFunction* pFunction, va_list list );

/**
*	Calls the given function using the given context.
*	@param pThis This pointer.
*	@param pFunction Function to call.
*	@param pContext Context to use.
*	@param flags Flags.
*	@param ... Arguments.
*/
bool CallMethod( void* pThis, asIScriptFunction* pFunction, asIScriptContext* pContext, CallFlags_t flags, ... );

/**
*	@see CallMethod( void* pThis, asIScriptFunction& function, asIScriptContext& context, CallFlags_t flags, ... )
*/
bool CallMethod( void* pThis, asIScriptFunction* pFunction, asIScriptContext* pContext, ... );

/**
*	Acquires a context using asIScriptEngine::RequestContext
*	@see CallMethod( void* pThis, asIScriptFunction& function, asIScriptContext& context, CallFlags_t flags, ... )
*/
bool CallMethod( void* pThis, asIScriptFunction* pFunction, CallFlags_t flags, ... );

/**
*	Acquires a context using asIScriptEngine::RequestContext
*	@see CallMethod( void* pThis, asIScriptFunction& function, asIScriptContext& context, CallFlags_t flags, ... )
*/
bool CallMethod( void* pThis, asIScriptFunction* pFunction, ... );
}

#endif //WRAPPER_CASCALLABLE_H