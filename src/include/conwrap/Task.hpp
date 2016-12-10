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
#include <utility>


namespace conwrap
{
	// forward declaration
	template <typename ResourceType>
	class Processor;
	template <typename ResourceType, template<typename ResourceType, typename ResultType> class TaskResultType>
	class ProcessorBase;
	template <typename ResourceType>
	class ProcessorProxy;

	// this is a bit quicker version for std::packaged_task
	template <typename ResourceType, typename FunctionType, template<typename ResourceType, typename ResultType> class TaskResultType>
	class Task
	{
		// friend declaration
		template <typename, template<typename, typename> class> friend class ProcessorBase;

		private:
			FunctionType                  fun;
			Processor<ResourceType>*      processorPtr;
			ProcessorProxy<ResourceType>* processorProxyPtr;
			std::promise<decltype(fun())> promise;

		public:
			explicit Task(FunctionType f, Processor<ResourceType>* p, ProcessorProxy<ResourceType>* pp)
			: fun(f)
			, processorPtr(p)
			, processorProxyPtr(pp) {}

			// copy contructor is required to comply with std::function<...>
			Task(Task& c)
			: fun(std::move(c.fun))
			, processorPtr(std::move(c.processorPtr))
			, processorProxyPtr(std::move(c.processorProxyPtr))
			, promise(std::move(c.promise)) {}

			Task(Task&& c)
			: fun(std::move(c.fun))
			, processorPtr(std::move(c.processorPtr))
			, processorProxyPtr(std::move(c.processorProxyPtr))
			, promise(std::move(c.promise)) {}

			inline void operator() ()
			{
				setPromiseValue(fun, promise);
			}

		protected:
			auto createResult() -> TaskResultType<ResourceType, decltype(fun())>
			{
				// this method can be called only once per task
				return TaskResultType<ResourceType, decltype(fun())>(processorPtr, processorProxyPtr, std::shared_future<decltype(fun())>(std::move(getFuture())));
			}

			inline auto getFuture()
			{
				return std::move(promise.get_future());
			}

			template <typename Fun, typename Fut>
			void setPromiseValue(Fun& f, std::promise<Fut>& p)
			{
				p.set_value(f());
			}

			template <typename Fun>
			void setPromiseValue(Fun& f, std::promise<void>& p)
			{
				f();
				p.set_value();
			}
	};
}
