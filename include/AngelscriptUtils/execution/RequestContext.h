#pragma once

#include <angelscript.h>

namespace asutils
{
/**
*	@brief Requests a context and holds it for the duration of the object's lifetime
*	Should be used on the stack only
*/
class RequestContext final
{
public:
	RequestContext(asIScriptEngine& engine)
		: m_Engine(engine)
		, m_Context(*engine.RequestContext())
	{
	}

	~RequestContext()
	{
		m_Engine.ReturnContext(&m_Context);
	}

	asIScriptEngine& GetEngine() { return m_Engine; }

	asIScriptContext& GetContext() { return m_Context; }

private:
	asIScriptEngine& m_Engine;
	asIScriptContext& m_Context;
};
}
