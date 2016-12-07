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
#include <conwrap/ContextContinuation.hpp>


namespace conwrap
{
	// forward declaration
	template <typename ResourceType>
	class Processor;
	template <typename ResourceType, template<typename ResourceType, typename ResultType> class TaskResultType>
	class ProcessorBase;
	template <typename ResourceType>
	class ProcessorProxy;

	template <typename ResourceType, typename ResultType, template<typename ResourceType, typename ResultType> class TaskResultType>
	class TaskResultBase
	{
		public:
			explicit TaskResultBase(ProcessorBase<ResourceType, TaskResultType>* p, ProcessorProxy<ResourceType>* pp, std::shared_future<ResultType> rf)
			: processorPtr(p)
			, processorProxyPtr(pp)
			, resultFuture(rf) {}

			template <typename F>
			auto then(F fun) -> TaskResultType<ResourceType, decltype(fun())>
			{
				return processorPtr->process([=]() -> decltype(fun())
				{
					return fun();
				});
			}

			template <typename F>
			auto then(F fun) -> TaskResultType<ResourceType, decltype(fun(std::declval<ContextContinuation<ResourceType, ResultType>>()))>
			{
				return processorPtr->process([f = fun, c = createContext()]() -> decltype(fun(std::declval<ContextContinuation<ResourceType, ResultType>>()))
				{
					return f(c);
				});
			}

		protected:
			inline ContextContinuation<ResourceType, ResultType> createContext()
			{
				// TODO: check ContextContinuation std::shared_future<ResultType>...
				return ContextContinuation<ResourceType, ResultType>(processorProxyPtr, resultFuture);
			}

			inline auto getResultFuture()
			{
				return resultFuture;
			}

		private:
			ProcessorBase<ResourceType, TaskResultType>* processorPtr;
			ProcessorProxy<ResourceType>*                processorProxyPtr;
			std::shared_future<ResultType>               resultFuture;
	};
}
