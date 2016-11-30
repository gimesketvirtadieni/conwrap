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
#include <conwrap/Handler.hpp>
#include <conwrap/HandlerWithContext.hpp>
#include <conwrap/HandlerWrapper.hpp>
#include <conwrap/Provider.hpp>
#include <conwrap/TaskResult.hpp>
#include <functional>
#include <future>


namespace conwrap
{
	template <typename ResourceType, template<typename ResourceType, typename ResultType> class TaskResultType>
	class ProcessorBase
	{
		public:
			virtual ResourceType* getResource() = 0;

			template <typename F>
			auto process(F fun) -> TaskResultType<ResourceType, decltype(fun())>
			{
				conwrap::Handler<F, decltype(fun()), ResourceType> handler(std::move(fun));
				auto future = handler.getFuture();

				// posting a new handler
				this->post(std::move(this->wrapHandler(std::move(handler))));

				return getProvider()->createTask(std::shared_future<decltype(fun())>(std::move(future)));
			}

			template <typename F>
			auto process(F fun) -> TaskResultType<ResourceType, decltype(fun(std::declval<Context<ResourceType>>()))>
			{
				conwrap::HandlerWithContext<F, decltype(fun(std::declval<Context<ResourceType>>())), ResourceType> handler(std::move(fun), getProvider()->createContext());
				auto future = handler.getFuture();

				// posting a new handler
				this->post(std::move(this->wrapHandler(std::move(handler))));

				return getProvider()->createTask(std::shared_future<decltype(fun(std::declval<Context<ResourceType>>()))>(std::move(future)));
			}

		protected:
			virtual Provider<ResourceType, TaskResultType>* getProvider() = 0;
			virtual void                                    post(HandlerWrapper) = 0;
			virtual HandlerWrapper                          wrapHandler(std::function<void()>) = 0;
			virtual HandlerWrapper                          wrapHandler(std::function<void()>, bool) = 0;
	};
}
