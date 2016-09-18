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
#include "HandlerContext.hpp"


template <typename ResourceType>
class Processor
{
	public:
		// TODO: figure out how to make this method protected
		inline HandlerContext<ResourceType> createHandlerContext()
		{
			return HandlerContext<ResourceType> (getResource(), this);
		}

		virtual ResourceType* getResource() = 0;

		virtual void flush() = 0;

		template <typename F>
		auto process(F fun) -> std::future<decltype(fun(createHandlerContext()))>
		{
			auto promisePtr = std::make_shared<std::promise<decltype(fun(createHandlerContext()))>>();

			// posting a new handler
			post([=]
			{
				setPromiseValue(*promisePtr, fun);
			});

			return promisePtr->get_future();
		}

	protected:
		virtual void post(std::function<void()>) = 0;

		template <typename Fut, typename Fun>
		void setPromiseValue(std::promise<Fut>& p, Fun& f)
		{
			p.set_value(f(createHandlerContext()));
		}

		template <typename Fun>
		void setPromiseValue(std::promise<void>& p, Fun& f)
		{
			f(createHandlerContext());
			p.set_value();
		}
};
