/*
 * Copyright 2016, Andrej Kislovskij
 *
 * This is PUBLIC DOMAIN software so use at your own risk as it comes
 * with no warranties. This code is yours to share, use and modify without
 * any restrictions or obligations.
 *
 * For more information see conwrap/LICENSE or refer refer to http://unlicense.org
 *
 * Author: gimesketvirtadieni at gmail dot com (Andrej Kislovskij)
 */

#pragma once

#include <future>


namespace conwrap
{
	// forward declaration
	template <typename ResourceType, template <typename ResourceType, typename ResultType> class TaskType>
	class ProcessorProxy;

	template <typename ResourceType, typename ResultType>
	class Task
	{
		private:
			ProcessorProxy<ResourceType, Task>* processorProxyPtr;
			std::shared_future<ResultType>      result;

		public:
			Task(ProcessorProxy<ResourceType, Task>* p, std::shared_future<ResultType> f)
			: processorProxyPtr(p)
			, result(f) {}

			Task(const Task& rhs)
			{
				processorProxyPtr = rhs.processorProxyPtr;
				result            = rhs.result;
			}

			virtual ~Task() {}

			Task& operator= (const Task& rhs)
			{
				if (&rhs != this)
				{
					processorProxyPtr = rhs.processorProxyPtr;
					result            = rhs.result;
				}
				return *this;
			}

			virtual ResultType getResult()
			{
				return result.get();
			}

			// TODO: this is work in progress
			template <typename F>
			auto then(F fun) -> Task<ResourceType, decltype(fun(result.get()))>
			{
				return processorProxyPtr->process([r = result, f = fun]() -> decltype(fun(result.get()))
				{
					return f(r.get());
				});
			}

			virtual void wait()
			{
				result.wait();
			}
	};
}
