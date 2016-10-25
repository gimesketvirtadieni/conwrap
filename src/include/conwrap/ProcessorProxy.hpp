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
	template <typename ResourceType, template<typename ResourceType> class ResultType>
	class ProcessorProxy
	{
		public:
			virtual ResourceType*                getResource() = 0;
			virtual void                         post(HandlerWrapper) = 0;
			virtual HandlerWrapper               wrapHandler(std::function<void()>) = 0;
			virtual HandlerWrapper               wrapHandler(std::function<void()>, bool) = 0;

			// TODO: figure out how to make this method protected
			virtual HandlerContext<ResourceType> createContext() = 0;

			template <typename F>
			auto process(F fun) -> ResultType<decltype(fun())>
			{
				auto promisePtr = std::make_shared<std::promise<decltype(fun())>>();

				// posting a new handler
				this->post(this->wrapHandler([=]
				{
					setPromiseValue(*promisePtr, fun);
				}));

				return ResultType<decltype(fun())>(std::shared_future<decltype(fun())>(promisePtr->get_future()));
			}

			template <typename F>
			auto process(F fun) -> ResultType<decltype(fun(createContext()))>
			{
				auto promisePtr = std::make_shared<std::promise<decltype(fun(createContext()))>>();

				// posting a new handler
				this->post(this->wrapHandler([=]
				{
					setPromiseValueWithContext(*promisePtr, fun);
				}));

				return ResultType<decltype(fun(createContext()))>(std::shared_future<decltype(fun(createContext()))>(promisePtr->get_future()));
			}

		protected:
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
	};
}
