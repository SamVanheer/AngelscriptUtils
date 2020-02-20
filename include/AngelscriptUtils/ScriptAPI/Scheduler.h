#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <angelscript.h>

#include "AngelscriptUtils/execution/Packing.h"
#include "AngelscriptUtils/utility/SmartPointers.h"

namespace asutils
{
/**
*	@brief Provides a means of scheduling functions for delayed execution
*	The functions scheduled must be part of the same module that is scheduling the function
*	Can be used both as a module-specific scheduler and a global scheduler
*	It is recommended to use one scheduler for each module
*/
class Scheduler final
{
private:
	static constexpr int REPEAT_INFINITE_TIMES = -1;

	static constexpr int32_t INVALID_ID = 0;

	enum class ScheduleType
	{
		Timeout,
		Interval,
		Repeating
	};

	using TimerID = uint32_t;

	/**
*	@brief Contains all of the information to make a scheduled call
*/
	struct ScheduledFunction final
	{
		ScheduledFunction(TimerID id,
			ReferencePointer<asIScriptFunction>&& function, ScriptParameters&& parameters, float executionTime, float repeatInterval, int repeatCount)
			: m_ID(id)
			, m_Function(std::move(function))
			, m_Parameters(std::move(parameters))
			, m_ExecutionTime(executionTime)
			, m_RepeatInterval(repeatInterval)
			, m_RepeatCount(repeatCount)
		{
		}

		ScheduledFunction(const ScheduledFunction&) = delete;
		ScheduledFunction& operator=(const ScheduledFunction&) = delete;

		const TimerID m_ID;

		const ReferencePointer<asIScriptFunction> m_Function;
		const ScriptParameters m_Parameters;

		float m_ExecutionTime;
		const float m_RepeatInterval;
		int m_RepeatCount;
	};

public:
	Scheduler() = default;
	~Scheduler() = default;

	Scheduler(const Scheduler&) = delete;
	Scheduler& operator=(const Scheduler&) = delete;

	/**
	*	@brief Sets the time used for timer base
	*/
	void SetCurrentTime(float value)
	{
		m_CurrentTime = value;
	}

	void ClearTimer(TimerID id);

	void Think(const float currentTime, asIScriptContext& context);

private:
	/**
	*	@brief Executes the given function with the given context
	*	@return Whether the function should be removed from the list
	*/
	bool ExecuteFunction(ScheduledFunction& function, asIScriptContext& context);

public:
	/**
	*	@brief Adjusts the execution times for every scheduled function by the given amount
	*/
	void AdjustExecutionTimes(float adjustAmount);

	/**
	*	@brief Removes all functions that are part of the given module
	*/
	void RemoveFunctionsOfModule(asIScriptModule& module);

	static void SetTimeout(asIScriptGeneric* parameters);

	static void SetInterval(asIScriptGeneric* parameters);

	static void SetRepeating(asIScriptGeneric* parameters);

private:
	void TrySetInterval(asIScriptGeneric& parameters, const ScheduleType scheduleType);

	/**
	*	@brief If the given object is a function, tries to schedule it with the given parameters
	*/
	TimerID TrySchedule(void* potentiallyAFunction, int typeId,
		asIScriptGeneric& parameters, asUINT startIndex, float repeatInterval, int repeatCount);

	/**
	*	@brief Schedules a given function with the given parameters and time
	*/
	TimerID Schedule(asIScriptFunction& function, ScriptParameters&& parameters, float executionTime, float repeatInterval, int repeatCount);

	static void WriteError(const char* message);

private:
	std::vector<std::unique_ptr<ScheduledFunction>> m_Functions;

	TimerID m_NextID = 1;

	float m_CurrentTime = 0;

	bool m_ExecutingFunctions = false;

	std::vector<std::unique_ptr<ScheduledFunction>> m_FunctionsToAdd;

	std::vector<TimerID> m_FunctionsToRemove;
};

/**
*	@brief Registers the Scheduler reference type and the TimerID typedef
*/
void RegisterSchedulerAPI(asIScriptEngine& engine);
}
