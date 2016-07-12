#ifndef WRAPPER_CASCONTEXT_H
#define WRAPPER_CASCONTEXT_H

#include <angelscript.h>

/**
*	A context that does not release the context in its destructor.
*/
class CASContext
{
public:
	CASContext( asIScriptContext& context );
	~CASContext() = default;

protected:
	CASContext() = default;

public:

	void Release();

	void ReleaseOwnership();

	operator bool() const;

	asIScriptContext* GetContext() { return m_pContext; }

protected:
	asIScriptContext* m_pContext = nullptr;

	asIScriptEngine* m_pEngine = nullptr;

private:
	CASContext( const CASContext& ) = delete;
	CASContext& operator=( const CASContext& ) = delete;
};

/**
*	RAII wrapper around a context that maintains ownership for its lifetime.
*/
class CASOwningContext final : public CASContext
{
public:
	CASOwningContext( asIScriptContext& context );
	CASOwningContext( asIScriptEngine& engine );
	~CASOwningContext();

protected:
	asIScriptContext* m_pContext = nullptr;

	asIScriptEngine* m_pEngine = nullptr;
};

#endif //WRAPPER_CASCONTEXT_H