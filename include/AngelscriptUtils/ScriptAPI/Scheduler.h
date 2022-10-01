#pragma once

#include <chrono>
#include <memory>
#include <vector>

#include <angelscript.h>

#include "AngelscriptUtils/utility/SmartPointers.h"

namespace asutils
{
/**
*	@brief Provides a means of scheduling functions for delayed execution
*/
class Scheduler final
{
private:
	/**
	*	@brief Contains all of the information to make a scheduled call
	*/
	struct ScheduledFunction final
	{
		ScheduledFunction(
			ReferencePointer<asIScriptFunction>&& function, float executionTime, float repeatInterval, int repeatCount)
			: m_Function(std::move(function))
			, m_ExecutionTime(executionTime)
			, m_RepeatInterval(repeatInterval)
			, m_RepeatCount(repeatCount)
		{
		}

		ScheduledFunction(const ScheduledFunction&) = delete;
		ScheduledFunction& operator=(const ScheduledFunction&) = delete;

		const ReferencePointer<asIScriptFunction> m_Function;

		// Used to avoid cases where the same memory address is used for a handle.
		// Stale handles won't be able to remove unrelated callbacks.
		const std::chrono::system_clock::time_point m_CreationTime{std::chrono::system_clock::now()};

		float m_ExecutionTime;
		const float m_RepeatInterval;
		int m_RepeatCount;
	};

public:
	const int REPEAT_INFINITE_TIMES = -1;

	struct ScheduledFunctionHandle
	{
		ScheduledFunction* Function{};
		std::chrono::system_clock::time_point CreationTime{};
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

	void Think(const float currentTime, asIScriptContext& context);

public:
	/**
	*	@brief Adjusts the execution times for every scheduled function by the given amount
	*/
	void AdjustExecutionTimes(float adjustAmount);

	/**
	*	@brief Removes all functions that are part of the given module
	*/
	void RemoveFunctionsOfModule(asIScriptModule& module);

	ScheduledFunctionHandle Schedule(asIScriptFunction* callback, float repeatInterval, int repeatCount);

	void ClearCallback(ScheduledFunctionHandle handle);

private:
	void ClearCallbackCore(const ScheduledFunctionHandle& handle);

	/**
	*	@brief Executes the given function with the given context
	*	@return Whether the function should be removed from the list
	*/
	bool ExecuteFunction(ScheduledFunction& function, asIScriptContext& context);

	static void WriteError(const char* message);

private:
	std::vector<std::unique_ptr<ScheduledFunction>> m_Functions;

	float m_CurrentTime = 0;

	bool m_ExecutingFunctions = false;

	std::vector<std::unique_ptr<ScheduledFunction>> m_FunctionsToAdd;

	std::vector<ScheduledFunctionHandle> m_FunctionsToRemove;
};

/**
*	@brief Registers the Scheduler reference type and the TimerID typedef
*/
void RegisterSchedulerAPI(asIScriptEngine& engine);
}
