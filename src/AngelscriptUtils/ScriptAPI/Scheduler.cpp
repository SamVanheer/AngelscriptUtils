#include <algorithm>
#include <iterator>

#include "AngelscriptUtils/ScriptAPI/Scheduler.h"
#include "AngelscriptUtils/utility/ContextUtils.h"

namespace asutils
{
void Scheduler::Think(const float currentTime, asIScriptContext& context)
{
	m_ExecutingFunctions = true;

	for (auto it = m_Functions.begin(); it != m_Functions.end();)
	{
		auto& function = **it;

		if (function.m_ExecutionTime <= currentTime &&
			ExecuteFunction(function, context))
		{
			it = m_Functions.erase(it);
		}
		else
		{
			++it;
		}
	}

	m_ExecutingFunctions = false;

	//Add new functions
	if (!m_FunctionsToAdd.empty())
	{
		if (m_Functions.empty())
		{
			m_Functions = std::move(m_FunctionsToAdd);
		}
		else
		{
			m_Functions.reserve(m_Functions.size() + m_FunctionsToAdd.size());
			std::move(m_FunctionsToAdd.begin(), m_FunctionsToAdd.end(), std::back_inserter(m_Functions));
			m_FunctionsToAdd.clear();
		}
	}

	//Remove functions flagged for removal
	//This can include functions that were added from m_FunctionsToAdd, so perform this operation after adding those
	for (auto& function : m_FunctionsToRemove)
	{
		ClearCallbackCore(function);
	}

	m_FunctionsToRemove.clear();
}

void Scheduler::AdjustExecutionTimes(float adjustAmount)
{
	for (auto& function : m_Functions)
	{
		function->m_ExecutionTime += adjustAmount;
	}
}

void Scheduler::RemoveFunctionsOfModule(asIScriptModule& module)
{
	m_Functions.erase(std::remove_if(m_Functions.begin(), m_Functions.end(), [&](const auto& candidate)
		{ return candidate->m_Function->GetModule() == &module; }), m_Functions.end());
}

Scheduler::ScheduledFunctionHandle Scheduler::Schedule(asIScriptFunction* callback, float repeatInterval, int repeatCount)
{
	if (!callback)
	{
		WriteError("Null callback passed to Schedule");
		return {};
	}

	ReferencePointer function{callback, true};

	{
		auto context = asGetActiveContext();

		auto caller = context->GetFunction();

		auto actualFunction = function.Get();

		if (auto delegate = actualFunction->GetDelegateFunction(); delegate)
		{
			actualFunction = delegate;
		}

		if (caller->GetModule() != actualFunction->GetModule())
		{
			WriteError("The given function must be part of the same script as the function that is scheduling the timer");
			return {};
		}
	}

	if (repeatCount == 0 || repeatCount < REPEAT_INFINITE_TIMES)
	{
		WriteError("Repeat count must be larger than zero or REPEAT_INFINITE_TIMES");
		return {};
	}

	// Allow 0 to execute a function every frame
	if (repeatInterval < 0)
	{
		WriteError("Repeat interval must be a positive value");
		return {};
	}

	auto& list = m_ExecutingFunctions ? m_FunctionsToAdd : m_Functions;

	list.push_back(std::make_unique<ScheduledFunction>(
		std::move(function), m_CurrentTime + repeatInterval, repeatInterval, repeatCount));

	auto scheduledFunction = list.back().get();

	return {scheduledFunction, scheduledFunction->m_CreationTime};
}

void Scheduler::ClearCallback(ScheduledFunctionHandle handle)
{
	if (!handle.Function)
	{
		WriteError("Null handle passed to ClearCallback");
		return;
	}

	if (m_ExecutingFunctions)
	{
		//Delay until functions have finished executing
		m_FunctionsToRemove.emplace_back(handle);
		return;
	}

	ClearCallbackCore(handle);
}

void Scheduler::ClearCallbackCore(const ScheduledFunctionHandle& handle)
{
	if (auto it = std::find_if(m_Functions.begin(), m_Functions.end(), [&](const auto& candidate)
		{ return candidate.get() == handle.Function && candidate->m_CreationTime == handle.CreationTime; }); it != m_Functions.end())
	{
		m_Functions.erase(it);
	}
	else
	{
		WriteError("Tried to clear callback that was already cleared");
	}
}

bool Scheduler::ExecuteFunction(ScheduledFunction& function, asIScriptContext& context)
{
	//Call will log any errors if the context has logging enabled
	auto result = context.Prepare(function.m_Function.Get());

	if (result >= 0)
	{
		result = context.Execute();
	}

	if (function.m_RepeatCount != REPEAT_INFINITE_TIMES)
	{
		--function.m_RepeatCount;
	}

	const auto shouldRemove = function.m_RepeatCount == 0;

	if (!shouldRemove)
	{
		//This must not use m_CurrentTime as a base because otherwise additional delays will creep in over time
		//This is because m_ExecutionTime is <= currentTime, not ==
		function.m_ExecutionTime += function.m_RepeatInterval;
	}

	return shouldRemove;
}

void Scheduler::WriteError(const char* message)
{
	const auto context = asGetActiveContext();
	const auto engine = context->GetEngine();

	LocationInfo info;
	GetCallerInfo(*context, info);

	engine->WriteMessage(info.section.c_str(), info.line, info.column, asMSGTYPE_ERROR, message);
}

static void ConstructScheduledFunctionHandle(void* memory)
{
	new (memory) Scheduler::ScheduledFunctionHandle();
}

static void DestructScheduledFunctionHandle(Scheduler::ScheduledFunctionHandle* handle)
{
	handle->~ScheduledFunctionHandle();
}

static void CopyConstructScheduledFunctionHandle(void* memory, const Scheduler::ScheduledFunctionHandle& other)
{
	new (memory) Scheduler::ScheduledFunctionHandle(other);
}

static bool ScheduledFunction_opImplConv(const Scheduler::ScheduledFunctionHandle* handle)
{
	return handle->Function != nullptr;
}

static void RegisterScheduledFunction(asIScriptEngine& engine)
{
	const char* const name = "ScheduledFunction";

	engine.RegisterObjectType(name, sizeof(Scheduler::ScheduledFunctionHandle), asOBJ_VALUE | asGetTypeTraits<Scheduler::ScheduledFunctionHandle>());

	engine.RegisterObjectBehaviour(name, asBEHAVE_CONSTRUCT, "void f()",
		asFUNCTION(ConstructScheduledFunctionHandle), asCALL_CDECL_OBJFIRST);

	engine.RegisterObjectBehaviour(name, asBEHAVE_DESTRUCT, "void f()",
		asFUNCTION(DestructScheduledFunctionHandle), asCALL_CDECL_OBJFIRST);

	engine.RegisterObjectBehaviour(name, asBEHAVE_CONSTRUCT, "void f(const ScheduledFunction& in other)",
		asFUNCTION(CopyConstructScheduledFunctionHandle), asCALL_CDECL_OBJFIRST);

	engine.RegisterObjectMethod(name, "ScheduledFunction& opAssign(const ScheduledFunction& in other)",
		asMETHODPR(Scheduler::ScheduledFunctionHandle, operator=, (const Scheduler::ScheduledFunctionHandle&), Scheduler::ScheduledFunctionHandle&),
		asCALL_THISCALL);

	engine.RegisterObjectMethod(name, "bool opImplConv() const",
		asFUNCTION(ScheduledFunction_opImplConv), asCALL_CDECL_OBJFIRST);
}

void RegisterSchedulerAPI(asIScriptEngine& engine)
{
	engine.RegisterFuncdef("void ScheduledCallback()");

	RegisterScheduledFunction(engine);

	const auto className = "ScriptScheduler";

	engine.RegisterObjectType(className, 0, asOBJ_REF | asOBJ_NOCOUNT);

	engine.RegisterObjectProperty(className, "const int REPEAT_INFINITE_TIMES", asOFFSET(Scheduler, REPEAT_INFINITE_TIMES));

	engine.RegisterObjectMethod(className,
		"ScheduledFunction Schedule(ScheduledCallback@ callback, float repeatInterval, int repeatCount = 1)",
		asMETHOD(Scheduler, Schedule), asCALL_THISCALL);

	engine.RegisterObjectMethod(className, "void ClearCallback(ScheduledFunction function)",
		asMETHOD(Scheduler, ClearCallback), asCALL_THISCALL);
}
}
