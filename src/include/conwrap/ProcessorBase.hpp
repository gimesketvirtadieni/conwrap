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

			// TODO: both process(...) methods should be combined, because only task type differs
			template <typename F>
			auto process(F fun) -> TaskResultType<ResourceType, decltype(fun())>
			{
				Handler<ResourceType, F, decltype(fun())> handler(std::move(fun));

				auto taskResult = getProvider()->createTaskResult(handler);

				// posting a new handler
				this->post(std::move(this->wrapHandler(std::move(handler))));

				return taskResult;
			}

			template <typename F>
			auto process(F fun) -> TaskResultType<ResourceType, decltype(fun(std::declval<Context<ResourceType>>()))>
			{
				HandlerWithContext<ResourceType, F, decltype(fun(std::declval<Context<ResourceType>>()))> handler(std::move(fun), getProvider()->createContext());

				auto taskResult = getProvider()->createTaskResult(handler);

				// posting a new handler
				this->post(std::move(this->wrapHandler(std::move(handler))));

				return taskResult;
			}

		protected:
			virtual Provider<ResourceType, TaskResultType>* getProvider() = 0;
			virtual void                                    post(HandlerWrapper) = 0;
			virtual HandlerWrapper                          wrapHandler(std::function<void()>) = 0;
			virtual HandlerWrapper                          wrapHandler(std::function<void()>, bool) = 0;
	};
}
