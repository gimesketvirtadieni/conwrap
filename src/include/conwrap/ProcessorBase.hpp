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

#include <conwrap/Context.hpp>
#include <conwrap/Task.hpp>
#include <conwrap/TaskResult.hpp>
#include <conwrap/TaskWithContext.hpp>
#include <conwrap/TaskWrapped.hpp>
#include <functional>
#include <future>


namespace conwrap
{
	// forward declaration
	template <typename ResourceType>
	class Processor;
	template <typename ResourceType>
	class ProcessorProxy;

	template <typename ResourceType, template<typename, typename> class TaskResultType>
	class ProcessorBase
	{
		public:
			virtual ResourceType* getResource() = 0;

			template <typename FunctionType>
			auto process(FunctionType fun) -> TaskResultType<ResourceType, decltype(fun())>
			{
				return std::move(processTask(std::move(Task<ResourceType, FunctionType, TaskResultType>(std::move(fun), getProcessor(), getProcessorProxy()))));
			}

			template <typename FunctionType>
			auto process(FunctionType fun) -> TaskResultType<ResourceType, decltype(fun(std::declval<Context<ResourceType>>()))>
			{
				return std::move(processTask(std::move(TaskWithContext<ResourceType, FunctionType, TaskResultType>(std::move(fun), Context<ResourceType>(getProcessorProxy()), getProcessor(), getProcessorProxy()))));
			}

		protected:
			virtual Processor<ResourceType>*      getProcessor() = 0;
			virtual ProcessorProxy<ResourceType>* getProcessorProxy() = 0;
			virtual void                          post(TaskWrapped) = 0;

			template <typename TaskType>
			auto processTask(TaskType task)
			{
				// creating and storing on the stack a task result so it can be returned to the caller
				auto taskResult = task.createResult();

				// posting the task for the processing
				this->post(std::move(this->wrap(std::move(task))));

				return std::move(taskResult);
			}

			virtual TaskWrapped wrap(std::function<void()>) = 0;
			virtual TaskWrapped wrap(std::function<void()>, bool) = 0;
	};
}
