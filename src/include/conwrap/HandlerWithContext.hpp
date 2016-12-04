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

	template <typename ResourceType, typename FunctionType, typename ResultType>
	class HandlerWithContext
	{
		// friend declaration
		template <typename, template<typename, typename> class> friend class Provider;

		private:
			FunctionType                                                       fun;
			std::promise<decltype(fun(std::declval<Context<ResourceType>>()))> promise;
			Context<ResourceType>                                              context;

		public:
			explicit HandlerWithContext(FunctionType f, Context<ResourceType> c)
			: fun(f)
			, context(c) {}

			HandlerWithContext(HandlerWithContext& c)
			: fun(std::move(c.fun))
			, promise(std::move(c.promise))
			, context(std::move(c.context)) {}

			HandlerWithContext(HandlerWithContext&& c)
			: fun(std::move(c.fun))
			, promise(std::move(c.promise))
			, context(std::move(c.context)) {}

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
