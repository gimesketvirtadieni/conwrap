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
#include <conwrap/ContinuationContext.hpp>


namespace conwrap
{
	// forward declaration
	template <typename ResourceType>
	class Processor;
	template <typename ResourceType, template<typename ResourceType, typename ResultType> class TaskType>
	class ProcessorBase;
	template <typename ResourceType>
	class ProcessorProxy;

	template <typename ResourceType, typename ResultType, template<typename ResourceType, typename ResultType> class TaskType>
	class TaskBase
	{
		public:
			TaskBase(ProcessorBase<ResourceType, TaskType>* p, ProcessorProxy<ResourceType>* pp, std::shared_future<ResultType> r)
			: processorPtr(p)
			, processorProxyPtr(pp)
			, result(r) {}

			virtual ~TaskBase() {}

			template <typename F>
			auto then(F fun) -> TaskType<ResourceType, decltype(fun())>
			{
				return processorPtr->process([=]() -> decltype(fun())
				{
					return fun();
				});
			}

			template <typename F>
			auto then(F fun) -> TaskType<ResourceType, decltype(fun(std::declval<ContinuationContext<ResourceType, ResultType>>()))>
			{
				return processorPtr->process([f = fun, c = createContext()]() -> decltype(fun(std::declval<ContinuationContext<ResourceType, ResultType>>()))
				{
					return f(c);
				});
			}

		protected:
			inline ContinuationContext<ResourceType, ResultType> createContext()
			{
				return ContinuationContext<ResourceType, ResultType>(processorProxyPtr, result);
			}

			inline std::shared_future<ResultType>* getResultFuture()
			{
				return &result;
			}

		private:
			ProcessorBase<ResourceType, TaskType>* processorPtr;
			ProcessorProxy<ResourceType>*          processorProxyPtr;
			std::shared_future<ResultType>         result;
	};
}
