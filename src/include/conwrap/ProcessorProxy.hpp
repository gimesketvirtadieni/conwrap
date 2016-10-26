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
#include <functional>
#include <conwrap/HandlerContext.hpp>
#include <conwrap/HandlerWrapper.hpp>


namespace conwrap
{
	template <typename ResourceType, template<typename ResourceType, typename ResultType> class TaskType>
	class ProcessorProxy
	{
		// this 'weird' struct is a workaround to get decltype work for protected method
		private:
			struct s
			{
				static HandlerContext<ResourceType> createContext() {};
			};

		public:
			virtual ResourceType* getResource() = 0;

			virtual void post(HandlerWrapper) = 0;

			template <typename F>
			auto process(F fun) -> TaskType<ResourceType, decltype(fun())>
			{
				auto promisePtr = std::make_shared<std::promise<decltype(fun())>>();

				// posting a new handler
				this->post(this->wrapHandler([=]
				{
					setPromiseValue(*promisePtr, fun);
				}));

				return TaskType<ResourceType, decltype(fun())>(this, std::shared_future<decltype(fun())>(promisePtr->get_future()));
			}

			template <typename F>
			auto process(F fun) -> TaskType<ResourceType, decltype(fun(s::createContext()))>
			{
				auto promisePtr = std::make_shared<std::promise<decltype(fun(s::createContext()))>>();

				// posting a new handler
				this->post(this->wrapHandler([=]
				{
					setPromiseValueWithContext(*promisePtr, fun);
				}));

				return TaskType<ResourceType, decltype(fun(s::createContext()))>(this, std::shared_future<decltype(fun(s::createContext()))>(promisePtr->get_future()));
			}

			virtual HandlerWrapper wrapHandler(std::function<void()>) = 0;

		protected:
			virtual HandlerContext<ResourceType> createContext() = 0;

			template <typename Fut, typename Fun>
			void setPromiseValue(std::promise<Fut>& p, Fun& f)
			{
				p.set_value(f());
			}

			template <typename Fun>
			void setPromiseValue(std::promise<void>& p, Fun& f)
			{
				f();
				p.set_value();
			}

			template <typename Fut, typename Fun>
			void setPromiseValueWithContext(std::promise<Fut>& p, Fun& f)
			{
				p.set_value(f(createContext()));
			}

			template <typename Fun>
			void setPromiseValueWithContext(std::promise<void>& p, Fun& f)
			{
				f(createContext());
				p.set_value();
			}

			virtual HandlerWrapper wrapHandler(std::function<void()>, bool) = 0;
	};
}
