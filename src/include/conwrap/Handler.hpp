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
	template <typename ResourceType, template<typename ResourceType, typename ResultType> class TaskResultType>
	class Provider;

	// this is a bit quicker version for std::packaged_task
	template <typename ResourceType, typename FunctionType, typename ResultType>
	class Handler
	{
		// friend declaration
		template <typename, template<typename, typename> class> friend class Provider;

		private:
			FunctionType                  fun;
			std::promise<decltype(fun())> promise;

		public:
			explicit Handler(FunctionType f)
			: fun(f) {}

			Handler(Handler& c)
			: fun(std::move(c.fun))
			, promise(std::move(c.promise)) {}

			Handler(Handler&& c)
			: fun(std::move(c.fun))
			, promise(std::move(c.promise)) {}

			// TODO: task should be connected with its result
/*
			auto createResult() -> TaskResultType<ResourceType, ResultType>
			{
				return TaskResultType<ResourceType, ResultType>(getProcessor(), getProcessorProxy(), task.getFuture());
			}
*/
			inline void operator() ()
			{
				setPromiseValue(fun, promise);
			}

		protected:
			inline auto getFuture()
			{
				return promise.get_future();
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
