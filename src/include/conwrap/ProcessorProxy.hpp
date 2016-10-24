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
#include <conwrap/ProcessorBase.hpp>
#include <conwrap/TaskProxy.hpp>


namespace conwrap
{
	// TODO: ProcessorProxy should be avoided by generalizing Processor template
	template <typename ResourceType>
	class ProcessorProxy : public internal::ProcessorBase<ResourceType>
	{
		public:
			// TODO: figure out how to make this method protected
			virtual HandlerContext<ResourceType> createHandlerContext() override = 0;

			template <typename F>
			auto process(F fun) -> TaskProxy<decltype(fun())>
			{
				auto promisePtr = std::make_shared<std::promise<decltype(fun())>>();

				// posting a new handler
				this->post(this->wrapHandler([=]
				{
					setPromiseValue(*promisePtr, fun);
				}));

				return TaskProxy<decltype(fun())>(std::shared_future<decltype(fun())>(promisePtr->get_future()));
			}

			template <typename F>
			auto process(F fun) -> TaskProxy<decltype(fun(createHandlerContext()))>
			{
				auto promisePtr = std::make_shared<std::promise<decltype(fun(createHandlerContext()))>>();

				// posting a new handler
				this->post(this->wrapHandler([=]
				{
					setPromiseValueWithContext(*promisePtr, fun);
				}));

				return TaskProxy<decltype(fun(createHandlerContext()))>(std::shared_future<decltype(fun(createHandlerContext()))>(promisePtr->get_future()));
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
				p.set_value(f(createHandlerContext()));
			}

			template <typename Fun>
			void setPromiseValueWithContext(std::promise<void>& p, Fun& f)
			{
				f(createHandlerContext());
				p.set_value();
			}
	};

}
