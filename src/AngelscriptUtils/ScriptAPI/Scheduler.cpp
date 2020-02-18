#include <limits>
#include <string>

#include "AngelscriptUtils/execution/Execution.h"

#include "AngelscriptUtils/ScriptAPI/Scheduler.h"

#include "AngelscriptUtils/utility/ContextUtils.h"
#include "AngelscriptUtils/utility/Objects.h"
#include "AngelscriptUtils/utility/RegistrationUtils.h"
#include "AngelscriptUtils/utility/TypeInfo.h"

namespace asutils
{
void Scheduler::ClearTimer(uint32_t id)
{
	if (m_CurrentFunctionID != INVALID_ID && m_CurrentFunctionID == id)
	{
		//Delay until function has finished executing so it stays alive long enough
		m_RemoveCurrentFunction = true;
		return;
	}

	auto end = m_Functions.end();

	for (auto it = m_Functions.begin(); it != end; ++it)
	{
		if ((*it)->m_ID == id)
		{
			m_Functions.erase(it);
			break;
		}
	}
}

void Scheduler::Think(const float currentTime, asIScriptContext& context)
{
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
}

bool Scheduler::ExecuteFunction(ScheduledFunction& function, asIScriptContext& context)
{
	m_CurrentFunctionID = function.m_ID;

	//Call will log any errors if the context has logging enabled
	PackedCall(*function.m_Function, context, function.m_Parameters);

	m_CurrentFunctionID = INVALID_ID;

	if (function.m_RepeatCount != REPEAT_INFINITE_TIMES)
	{
		--function.m_RepeatCount;
	}

	const auto shouldRemove = m_RemoveCurrentFunction || (function.m_RepeatCount == 0);

	m_RemoveCurrentFunction = false;

	if (!shouldRemove)
	{
		//This must not use m_CurrentTime as a base because otherwise additional delays will creep in over time
		//This is because m_ExecutionTime is <= currentTime, not ==
		function.m_ExecutionTime += function.m_RepeatInterval;
	}

	return shouldRemove;
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
	for (auto it = m_Functions.begin(); it != m_Functions.end();)
	{
		if ((*it)->m_Function->GetModule() == &module)
		{
			it = m_Functions.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void Scheduler::SetTimeout(asIScriptGeneric* parameters)
{
	auto& scheduler = *reinterpret_cast<Scheduler*>(parameters->GetObject());

	scheduler.TrySetInterval(*parameters, ScheduleType::Timeout);
}

void Scheduler::SetInterval(asIScriptGeneric* parameters)
{
	auto& scheduler = *reinterpret_cast<Scheduler*>(parameters->GetObject());

	scheduler.TrySetInterval(*parameters, ScheduleType::Interval);
}

void Scheduler::SetRepeating(asIScriptGeneric* parameters)
{
	auto& scheduler = *reinterpret_cast<Scheduler*>(parameters->GetObject());

	scheduler.TrySetInterval(*parameters, ScheduleType::Repeating);
}

void Scheduler::TrySetInterval(asIScriptGeneric& parameters, const ScheduleType scheduleType)
{
	const auto potentiallyAFunction = parameters.GetArgAddress(0);
	const auto typeId = parameters.GetArgTypeId(0);

	const auto repeatInterval = parameters.GetArgFloat(1);

	int repeatCount;

	switch (scheduleType)
	{
	case ScheduleType::Timeout:
		//Repeat once if timeout
		repeatCount = 1;
		break;

	case ScheduleType::Interval:
		repeatCount = static_cast<int>(parameters.GetArgDWord(2));
		break;

		//Default case should never be hit but the compiler doesn't agree
	default:
	case ScheduleType::Repeating:
		//Repeat until caller clears timer
		repeatCount = REPEAT_INFINITE_TIMES;
		break;
	}

	const asUINT startIndex = scheduleType == ScheduleType::Interval ? 3 : 2;

	const auto timerID = TrySchedule(potentiallyAFunction, typeId, parameters, startIndex, repeatInterval, repeatCount);

	parameters.SetReturnDWord(timerID);
}

uint32_t Scheduler::TrySchedule(void* potentiallyAFunction, int typeId,
	asIScriptGeneric& parameters, asUINT startIndex, float repeatInterval, int repeatCount)
{
	auto& engine = *parameters.GetEngine();

	auto function = TryGetFunctionFromVariableParameter(engine, potentiallyAFunction, typeId);

	if (!function)
	{
		WriteError("Function parameter must be a function or function delegate");
		return INVALID_ID;
	}

	{
		auto context = asGetActiveContext();

		auto caller = context->GetFunction(context->GetCallstackSize() - 1);

		if (caller->GetModule() != function->GetModule())
		{
			WriteError("The given function must be part of the same script as the function that is scheduling the timer");
			return INVALID_ID;
		}
	}

	//This is not off by one; if there are no variadic parameters then startIndex will be equal to the parameter count
	if (startIndex > static_cast<asUINT>(parameters.GetArgCount()))
	{
		WriteError("Application error: starting index must be in range");
		return INVALID_ID;
	}

	const asUINT parameterCount = parameters.GetArgCount() - startIndex;

	//Validate the parameters
	if (function->GetParamCount() != parameterCount)
	{
		WriteError("Parameter count must match the target function");
		return INVALID_ID;
	}

	//Validate repeat settings
	//Allow 0 to execute a function every frame
	if (repeatInterval < 0)
	{
		WriteError("Repeat interval must be a positive value");
		return INVALID_ID;
	}

	if (repeatCount <= 0 && repeatCount != REPEAT_INFINITE_TIMES)
	{
		WriteError("Repeat count must be larger than zero or REPEAT_INFINITE_TIMES");
		return INVALID_ID;
	}

	//Construct parameter list and convert types to match exactly the target function
	//This is more efficient than converting on execution
	ScriptParameters parameterList;

	try
	{
		parameterList = CreateGenericParameterList(parameters, startIndex);
	}
	catch (const ScriptParameterErrorException& e)
	{
		WriteError((std::string{"Application error: Error creating parameter list: "} + e.what()).c_str());
		return INVALID_ID;
	}

	//TODO: modify parameters to match exactly the target function

	return Schedule(*function, std::move(parameterList), m_CurrentTime + repeatInterval, repeatInterval, repeatCount);
}

uint32_t Scheduler::Schedule(asIScriptFunction& function, ScriptParameters&& parameters, float executionTime, float repeatInterval, int repeatCount)
{
	const auto timerID = m_NextID++;

	//Unlikely to happen but will occur on long running scripts
	if (m_NextID == std::numeric_limits<uint32_t>::max())
	{
		m_NextID = 1;
	}

	auto scheduled = std::make_unique<ScheduledFunction>(timerID,
		ReferencePointer<asIScriptFunction>(&function), std::move(parameters), executionTime, repeatInterval, repeatCount);

	m_Functions.emplace_back(std::move(scheduled));

	return timerID;
}

void Scheduler::WriteError(const char* message)
{
	auto context = asGetActiveContext();

	auto& engine = *context->GetEngine();

	LocationInfo info;

	GetCallerInfo(*context, info);

	engine.WriteMessage(info.section.c_str(), info.line, info.column, asMSGTYPE_ERROR, message);
}

void RegisterSchedulerAPI(asIScriptEngine& engine)
{
	const auto className = "ScriptScheduler";

	const size_t MaxVarArgs = 8;

	engine.RegisterObjectType(className, 0, asOBJ_REF | asOBJ_NOCOUNT);

	//Use a typedef for this to make it possible to change the underlying type to 64 bit if it's ever needed
	engine.RegisterTypedef("TimerID", "uint32");

	RegisterVariadicMethod(engine, className, "TimerID", "SetTimeout", "?& in function, float delay", 0, MaxVarArgs, asFUNCTION(Scheduler::SetTimeout));
	RegisterVariadicMethod(engine, className, "TimerID", "SetInterval", "?& in function, float repeatInterval, int repeatCount", 0, MaxVarArgs, asFUNCTION(Scheduler::SetInterval));
	RegisterVariadicMethod(engine, className, "TimerID", "SetRepeating", "?& in function, float repeatInterval", 0, MaxVarArgs, asFUNCTION(Scheduler::SetRepeating));

	engine.RegisterObjectMethod(className, "void ClearTimer(TimerID id)", asMETHOD(Scheduler, ClearTimer), asCALL_THISCALL);
}
}
