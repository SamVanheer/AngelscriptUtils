#ifndef WRAPPER_CASCALLABLE_H
#define WRAPPER_CASCALLABLE_H

#include <cstdarg>
#include <cstdint>

#include <angelscript.h>

class CASContext;
class CASArguments;

typedef uint32_t CallFlags_t;

namespace CallFlag
{
/**
*	Flags to affect function calls.
*/
enum CallFlag : CallFlags_t
{
	/**
	*	No flags.
	*/
	NONE = 0,
};
}

/**
*	Base class for callable types.
*/
class CASCallable
{
protected:
	template<typename CALLABLE, typename ARGS>
	friend bool CallFunction( CALLABLE& callable, CallFlags_t flags, const ARGS& args );

protected:
	/**
	*	Constructor.
	*	@param function Function to call.
	*	@param context Context to use for calls.
	*/
	CASCallable( asIScriptFunction& function, CASContext& context );

public:
	/**
	*	@return The function.
	*/
	asIScriptFunction& GetFunction() { return m_Function; }

	/**
	*	@return The context.
	*/
	CASContext& GetContext() { return m_Context; }

	/**
	*	@return Whether this function is valid.
	*/
	bool IsValid() const;

	/**
	*	Gets the return value.
	*	@param pReturnValue Pointer to the variable that will receive the return value. Must match the type being retrieved.
	*	@return true if the value was successfully retrieved, false otherwise.
	*/
	bool GetReturnValue( void* pReturnValue );

protected:
	/**
	*	Called before the arguments are set. Lets the callable type evaluate itself.
	*	Non-virtual, uses templates to invoke the correct type.
	*	@return true if the call should continue, false otherwise.
	*/
	bool PreSetArguments() { return true; }

	/**
	*	Called after arguments are set, before the function is executed. Lets the callable type evaluate itself.
	*	Non-virtual, uses templates to invoke the correct type.
	*	@return true if the call should continue, false otherwise.
	*/
	bool PreExecute() { return true; }

	/**
	*	Called after the function is executed. Lets the callable type evaluate itself.
	*	Non-virtual, uses templates to invoke the correct type.
	*	@param iResult Result of the asIScriptContext::Execute call.
	*	@return true if the call should continue, false otherwise.
	*/
	bool PostExecute( const int iResult ) { return true; }

private:
	asIScriptFunction& m_Function;
	CASContext& m_Context;

private:
	CASCallable( const CASCallable& ) = delete;
	CASCallable& operator=( const CASCallable& ) = delete;
};

/**
*	A regular function.
*/
class CASFunction final : public CASCallable
{
public:
	CASFunction( asIScriptFunction& function, CASContext& context )
		: CASCallable( function, context )
	{
	}

	/**
	*	Calls the function.
	*	@param flags Call flags.
	*	@param list Pointer to a va_list that contains the arguments for the function.
	*	@return true on success, false otherwise.
	*/
	bool VCall( CallFlags_t flags, va_list list );

	/**
	*	Calls the function.
	*	@param flags Call flags.
	*	@param ... The arguments for the function.
	*	@return true on success, false otherwise.
	*/
	bool operator()( CallFlags_t flags, ... );

	/**
	*	Calls the function.
	*	@param flags Call flags.
	*	@param args List of arguments.
	*	@return true on success, false otherwise.
	*/
	bool CallArgs( CallFlags_t flags, const CASArguments& args );
};

/**
*	An object method.
*/
class CASMethod final : public CASCallable
{
protected:
	template<typename CALLABLE, typename ARGS>
	friend bool CallFunction( CALLABLE& callable, CallFlags_t flags, const ARGS& args );

public:
	/**
	*	@copydoc CASCallable::CASCallable( asIScriptFunction& function, CASContext& context )
	*	@param pThis Pointer to the object instance.
	*/
	CASMethod( asIScriptFunction& function, CASContext& context, void* pThis );

	bool IsValid() const;

	/**
	*	Calls the function.
	*	@param flags Call flags.
	*	@param list Pointer to a va_list that contains the arguments for the function.
	*	@return true on success, false otherwise.
	*/
	bool VCall( CallFlags_t flags, va_list list );

	/**
	*	Calls the function.
	*	@param flags Call flags.
	*	@param ... The arguments for the function.
	*	@return true on success, false otherwise.
	*/
	bool operator()( CallFlags_t flags, ... );

	/**
	*	Calls the function.
	*	@param flags Call flags.
	*	@param args List of arguments.
	*	@return true on success, false otherwise.
	*/
	bool CallArgs( CallFlags_t flags, const CASArguments& args );

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
*	@see VCallFunction( asIScriptFunction* pFunction, asIScriptContext* pContext, CallFlags_t flags, va_list list )
*/
bool VCallFunction( asIScriptFunction* pFunction, asIScriptContext* pContext, va_list list );

/**
*	Acquires a context using asIScriptEngine::RequestContext
*	@see VCallFunction( asIScriptFunction* pFunction, asIScriptContext* pContext, CallFlags_t flags, va_list list )
*/
bool VCallFunction( asIScriptFunction* pFunction, CallFlags_t flags, va_list list );

/**
*	Acquires a context using asIScriptEngine::RequestContext
*	@see VCallFunction( asIScriptFunction* pFunction, asIScriptContext* pContext, CallFlags_t flags, va_list list )
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
*	@see CallFunction( asIScriptFunction* pFunction, asIScriptContext* pContext, CallFlags_t flags, ... )
*/
bool CallFunction( asIScriptFunction* pFunction, asIScriptContext* pContext, ... );

/**
*	Acquires a context using asIScriptEngine::RequestContext
*	@see CallFunction( asIScriptFunction* pFunction, asIScriptContext* pContext, CallFlags_t flags, ... )
*/
bool CallFunction( asIScriptFunction* pFunction, CallFlags_t flags, ... );

/**
*	Acquires a context using asIScriptEngine::RequestContext
*	@see CallFunction( asIScriptFunction* pFunction, asIScriptContext* pContext, CallFlags_t flags, ... )
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
*	@see VCallMethod( void* pThis, asIScriptFunction* pFunction, asIScriptContext* pContext, CallFlags_t flags, va_list list )
*/
bool VCallMethod( void* pThis, asIScriptFunction* pFunction, asIScriptContext* pContext, va_list list );

/**
*	Acquires a context using asIScriptEngine::RequestContext
*	@see VCallMethod( void* pThis, asIScriptFunction* pFunction, asIScriptContext* pContext, CallFlags_t flags, va_list list )
*/
bool VCallMethod( void* pThis, asIScriptFunction* pFunction, CallFlags_t flags, va_list list );

/**
*	Acquires a context using asIScriptEngine::RequestContext
*	@see VCallMethod( void* pThis, asIScriptFunction* pFunction, asIScriptContext* pContext, CallFlags_t flags, va_list list )
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
*	@see CallMethod( void* pThis, asIScriptFunction* pFunction, asIScriptContext* pContext, CallFlags_t flags, ... )
*/
bool CallMethod( void* pThis, asIScriptFunction* pFunction, asIScriptContext* pContext, ... );

/**
*	Acquires a context using asIScriptEngine::RequestContext
*	@see CallMethod( void* pThis, asIScriptFunction* pFunction, asIScriptContext* pContext, CallFlags_t flags, ... )
*/
bool CallMethod( void* pThis, asIScriptFunction* pFunction, CallFlags_t flags, ... );

/**
*	Acquires a context using asIScriptEngine::RequestContext
*	@see CallMethod( void* pThis, asIScriptFunction* pFunction, asIScriptContext* pContext, CallFlags_t flags, ... )
*/
bool CallMethod( void* pThis, asIScriptFunction* pFunction, ... );
}

#endif //WRAPPER_CASCALLABLE_H