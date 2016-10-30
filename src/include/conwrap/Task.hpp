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
#include <conwrap/HandlerContext.hpp>


namespace conwrap
{
	// forward declaration
	template <typename ResourceType>
	class Processor;
	template <typename ResourceType>
	class ProcessorProxy;

	template <typename ResourceType, typename ResultType>
	class Task
	{
		// this 'weird' struct is a workaround to get decltype work for protected method
		private:
			struct s
			{
				static ContinuationContext<ResourceType, ResultType> cc();
			};

		public:
			Task(Processor<ResourceType>* p, ProcessorProxy<ResourceType>* pp, std::shared_future<ResultType> r)
			: processorPtr(p)
			, processorProxyPtr(pp)
			, result(r) {}

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

			inline ResultType getResult()
			{
				return result.get();
			}

			template <typename F>
			auto then(F fun) -> Task<ResourceType, decltype(fun())>
			{
				return processorPtr->process([=]() -> decltype(fun())
				{
					return fun();
				});
			}

			template <typename F>
			auto then(F fun) -> Task<ResourceType, decltype(fun(s::cc()))>
			{
				return processorPtr->process([f = fun, c = createContext()]() -> decltype(fun(s::cc()))
				{
					return f(c);
				});
			}

			inline void wait()
			{
				result.wait();
			}

		protected:
			inline ContinuationContext<ResourceType, ResultType> createContext()
			{
				return ContinuationContext<ResourceType, ResultType>(processorProxyPtr, result);
			}

		private:
			Processor<ResourceType>*       processorPtr;
			ProcessorProxy<ResourceType>*  processorProxyPtr;
			std::shared_future<ResultType> result;
	};
}
