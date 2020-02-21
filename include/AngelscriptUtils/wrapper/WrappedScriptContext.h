#pragma once

#include <memory>
#include <string_view>

#include <angelscript.h>

#include <spdlog/spdlog.h>

namespace asutils
{
/**
*	@brief Wraps a script context instance
*/
class WrappedScriptContext : public asIScriptContext
{
public:
	WrappedScriptContext(asIScriptContext& context)
		: m_Context(context)
	{
		m_Context.AddRef();
	}

	virtual ~WrappedScriptContext()
	{
		m_Context.Release();
	}

	WrappedScriptContext(const WrappedScriptContext&) = delete;
	WrappedScriptContext& operator=(const WrappedScriptContext&) = delete;

	// Memory management
	int AddRef() const override final
	{
		return asAtomicInc(m_RefCount);
	}

	int Release() const override final
	{
		const auto result = asAtomicDec(m_RefCount);

		if (result == 0)
		{
			delete this;
		}

		return result;
	}

	// Miscellaneous
	asIScriptEngine* GetEngine() const override { return m_Context.GetEngine(); }

	// Execution
	int             Prepare(asIScriptFunction* func) override { return m_Context.Prepare(func); }
	int             Unprepare() override { return m_Context.Unprepare(); }
	int             Execute() override { return m_Context.Execute(); }
	int             Abort() override { return m_Context.Abort(); }
	int             Suspend() override { return m_Context.Suspend(); }
	asEContextState GetState() const override { return m_Context.GetState(); }
	int             PushState() override { return m_Context.PushState(); }
	int             PopState() override { return m_Context.PopState(); }
	bool            IsNested(asUINT* nestCount = 0) const override { return m_Context.IsNested(nestCount); }

	// Object pointer for calling class methods
	int   SetObject(void* obj) override { return m_Context.SetObject(obj); }

	// Arguments
	int   SetArgByte(asUINT arg, asBYTE value) override { return m_Context.SetArgByte(arg, value); }
	int   SetArgWord(asUINT arg, asWORD value) override { return m_Context.SetArgWord(arg, value); }
	int   SetArgDWord(asUINT arg, asDWORD value) override { return m_Context.SetArgDWord(arg, value); }
	int   SetArgQWord(asUINT arg, asQWORD value) override { return m_Context.SetArgQWord(arg, value); }
	int   SetArgFloat(asUINT arg, float value) override { return m_Context.SetArgFloat(arg, value); }
	int   SetArgDouble(asUINT arg, double value) override { return m_Context.SetArgDouble(arg, value); }
	int   SetArgAddress(asUINT arg, void* addr) override { return m_Context.SetArgAddress(arg, addr); }
	int   SetArgObject(asUINT arg, void* obj) override { return m_Context.SetArgObject(arg, obj); }
	int   SetArgVarType(asUINT arg, void* ptr, int typeId) override { return m_Context.SetArgVarType(arg, ptr, typeId); }
	void* GetAddressOfArg(asUINT arg) override { return m_Context.GetAddressOfArg(arg); }

	// Return value
	asBYTE  GetReturnByte() override { return m_Context.GetReturnByte(); }
	asWORD  GetReturnWord() override { return m_Context.GetReturnWord(); }
	asDWORD GetReturnDWord() override { return m_Context.GetReturnDWord(); }
	asQWORD GetReturnQWord() override { return m_Context.GetReturnQWord(); }
	float   GetReturnFloat() override { return m_Context.GetReturnFloat(); }
	double  GetReturnDouble() override { return m_Context.GetReturnDouble(); }
	void* GetReturnAddress() override { return m_Context.GetReturnAddress(); }
	void* GetReturnObject() override { return m_Context.GetReturnObject(); }
	void* GetAddressOfReturnValue() override { return m_Context.GetAddressOfReturnValue(); }

	// Exception handling
	int                SetException(const char* info, bool allowCatch = true) override { return m_Context.SetException(info, allowCatch); }
	int                GetExceptionLineNumber(int* column = 0, const char** sectionName = 0) override
	{
		return m_Context.GetExceptionLineNumber(column, sectionName);
	}

	asIScriptFunction* GetExceptionFunction() override { return m_Context.GetExceptionFunction(); }
	const char* GetExceptionString() override { return m_Context.GetExceptionString(); }
	bool               WillExceptionBeCaught() override { return m_Context.WillExceptionBeCaught(); }
	int                SetExceptionCallback(asSFuncPtr callback, void* obj, int callConv) override
	{
		return m_Context.SetExceptionCallback(callback, obj, callConv);
	}

	void               ClearExceptionCallback() override { m_Context.ClearExceptionCallback(); }

	// Debugging
	int                SetLineCallback(asSFuncPtr callback, void* obj, int callConv) override { return m_Context.SetLineCallback(callback, obj, callConv); }
	void               ClearLineCallback() override { return m_Context.ClearLineCallback(); }
	asUINT             GetCallstackSize() const override { return m_Context.GetCallstackSize(); }
	asIScriptFunction* GetFunction(asUINT stackLevel = 0) override { return m_Context.GetFunction(stackLevel); }
	int                GetLineNumber(asUINT stackLevel = 0, int* column = 0, const char** sectionName = 0) override
	{
		return m_Context.GetLineNumber(stackLevel, column, sectionName);
	}

	int                GetVarCount(asUINT stackLevel = 0) override { return m_Context.GetVarCount(stackLevel); }
	const char* GetVarName(asUINT varIndex, asUINT stackLevel = 0) override { return m_Context.GetVarName(varIndex, stackLevel); }
	const char* GetVarDeclaration(asUINT varIndex, asUINT stackLevel = 0, bool includeNamespace = false) override
	{
		return m_Context.GetVarDeclaration(varIndex, stackLevel, includeNamespace);
	}

	int                GetVarTypeId(asUINT varIndex, asUINT stackLevel = 0) override { return m_Context.GetVarTypeId(varIndex, stackLevel); }
	void* GetAddressOfVar(asUINT varIndex, asUINT stackLevel = 0) override { return m_Context.GetAddressOfVar(varIndex, stackLevel); }
	bool               IsVarInScope(asUINT varIndex, asUINT stackLevel = 0) override { return m_Context.IsVarInScope(varIndex, stackLevel); }
	int                GetThisTypeId(asUINT stackLevel = 0) override { return m_Context.GetThisTypeId(stackLevel); }
	void* GetThisPointer(asUINT stackLevel = 0) override { return m_Context.GetThisPointer(stackLevel); }
	asIScriptFunction* GetSystemFunction() override { return m_Context.GetSystemFunction(); }

	// User data
	virtual void* SetUserData(void* data, asPWORD type = 0) override { return m_Context.SetUserData(data, type);  }
	virtual void* GetUserData(asPWORD type = 0) const override { return m_Context.GetUserData(type); }

protected:
	asIScriptContext& m_Context;
	mutable int m_RefCount = 1;
};

/**
*	@brief Logs errors that occur during execution
*/
class LoggingScriptContext : public WrappedScriptContext
{
public:
	LoggingScriptContext(asIScriptContext& context, const std::shared_ptr<spdlog::logger>& logger, bool suspendIsError = false)
		: WrappedScriptContext(context)
		, m_Logger(logger)
		, m_SuspendIsError(suspendIsError)
	{
	}

	int Execute() override;

private:
	void LogCurrentFunction(const std::string_view action);

private:
	const std::shared_ptr<spdlog::logger> m_Logger;
	const bool m_SuspendIsError;
};
}
