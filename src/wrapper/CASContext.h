#ifndef WRAPPER_CASCONTEXT_H
#define WRAPPER_CASCONTEXT_H

#include <angelscript.h>

/**
*	RAII wrapper around a context that maintains ownership for its lifetime.
*/
class CASContext
{
public:
	CASContext( asIScriptContext& context );
	CASContext( asIScriptEngine& engine );
	~CASContext();

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
*	A context that does not release the context in its destructor.
*/
class CASNonOwningContext final : public CASContext
{
public:
	using CASContext::CASContext;

	~CASNonOwningContext();
};

#endif //WRAPPER_CASCONTEXT_H