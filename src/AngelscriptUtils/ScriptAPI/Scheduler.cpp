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
		ClearCallbackCore(function.Get());
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

asIScriptFunction* Scheduler::Schedule(asIScriptFunction* callback, float repeatInterval, int repeatCount)
{
	if (!callback)
	{
		WriteError("Null callback passed");
		return nullptr;
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
			return nullptr;
		}
	}

	if (repeatCount == 0 || repeatCount < REPEAT_INFINITE_TIMES)
	{
		WriteError("Repeat count must be larger than zero or REPEAT_INFINITE_TIMES");
		return nullptr;
	}

	// Allow 0 to execute a function every frame
	if (repeatInterval < 0)
	{
		WriteError("Repeat interval must be a positive value");
		return nullptr;
	}

	auto& list = m_ExecutingFunctions ? m_FunctionsToAdd : m_Functions;

	list.push_back(std::make_unique<ScheduledFunction>(
		ReferencePointer<asIScriptFunction>(function), m_CurrentTime + repeatInterval, repeatInterval, repeatCount));

	// Caller gets a strong reference.
	return function.Release();
}

void Scheduler::ClearCallback(asIScriptFunction* callback)
{
	ReferencePointer<asIScriptFunction> cleanup{callback, true};

	if (m_ExecutingFunctions)
	{
		//Delay until functions have finished executing
		m_FunctionsToRemove.emplace_back(std::move(cleanup));
		return;
	}

	ClearCallbackCore(callback);
}

void Scheduler::ClearCallbackCore(asIScriptFunction* callback)
{
	if (auto it = std::find_if(m_Functions.begin(), m_Functions.end(), [&](const auto& candidate)
		{ return candidate->m_Function.Get() == callback; }); it != m_Functions.end())
	{
		m_Functions.erase(it);
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

void RegisterSchedulerAPI(asIScriptEngine& engine)
{
	engine.RegisterFuncdef("void ScheduledCallback()");

	const auto className = "ScriptScheduler";

	engine.RegisterObjectType(className, 0, asOBJ_REF | asOBJ_NOCOUNT);

	engine.RegisterObjectProperty(className, "const int REPEAT_INFINITE_TIMES", asOFFSET(Scheduler, REPEAT_INFINITE_TIMES));

	engine.RegisterObjectMethod(className,
		"ScheduledCallback@ Schedule(ScheduledCallback@ callback, float repeatInterval, int repeatCount = 1)",
		asMETHOD(Scheduler, Schedule), asCALL_THISCALL);

	engine.RegisterObjectMethod(className, "void ClearCallback(ScheduledCallback@ callback)", asMETHOD(Scheduler, ClearCallback), asCALL_THISCALL);
}
}
