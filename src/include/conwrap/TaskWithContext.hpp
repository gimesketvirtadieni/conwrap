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
#include <future>
#include <utility>


namespace conwrap
{
	// forward declaration
	template <typename ResourceType>
	class Processor;
	template <typename ResourceType, template<typename, typename> class TaskResultType>
	class ProcessorBase;
	template <typename ResourceType>
	class ProcessorProxy;

	template <typename ResourceType, typename FunctionType, template<typename, typename> class TaskResultType>
	class TaskWithContext
	{
		// friend declaration
		template <typename, template<typename, typename> class> friend class ProcessorBase;

		private:
			FunctionType                                                       fun;
			Context<ResourceType>                                              context;
			Processor<ResourceType>*                                           processorPtr;
			ProcessorProxy<ResourceType>*                                      processorProxyPtr;
			std::promise<decltype(fun(std::declval<Context<ResourceType>>()))> promise;

		public:
			explicit TaskWithContext(FunctionType f, Context<ResourceType> c, Processor<ResourceType>* p, ProcessorProxy<ResourceType>* pp)
			: fun(f)
			, context(c)
			, processorPtr(p)
			, processorProxyPtr(pp) {}

			// copy contructor is required to comply with std::function<...>
			TaskWithContext(TaskWithContext& c)
			: fun(std::move(c.fun))
			, context(std::move(c.context))
			, processorPtr(std::move(c.processorPtr))
			, processorProxyPtr(std::move(c.processorProxyPtr))
			, promise(std::move(c.promise)) {}

			TaskWithContext(TaskWithContext&& c)
			: fun(std::move(c.fun))
			, context(std::move(c.context))
			, processorPtr(std::move(c.processorPtr))
			, processorProxyPtr(std::move(c.processorProxyPtr))
			, promise(std::move(c.promise)) {}

			inline void operator() ()
			{
				setPromiseValue(fun, promise);
			}

		protected:
			auto createResult() -> TaskResultType<ResourceType, decltype(fun(std::declval<Context<ResourceType>>()))>
			{
				// this method can be called only once per task
				return TaskResultType<ResourceType, decltype(fun(std::declval<Context<ResourceType>>()))>(processorPtr, processorProxyPtr, std::shared_future<decltype(fun(std::declval<Context<ResourceType>>()))>(std::move(getFuture())));
			}

			inline auto getFuture()
			{
				return std::move(promise.get_future());
			}

			template <typename Fun, typename Fut>
			void setPromiseValue(Fun& f, std::promise<Fut>& p)
			{
				p.set_value(f(context));
			}

			template <typename Fun>
			void setPromiseValue(Fun& f, std::promise<void>& p)
			{
				f(context);
				p.set_value();
			}
	};
}
