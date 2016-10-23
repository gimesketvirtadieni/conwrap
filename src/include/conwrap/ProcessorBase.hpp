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

#include <functional>
#include <conwrap/HandlerContext.hpp>
#include <conwrap/HandlerWrapper.hpp>


namespace conwrap
{
	namespace internal
	{
		template <typename ResourceType>
		class ProcessorBase
		{
			public:
				virtual ResourceType* getResource() = 0;
				virtual void post(HandlerWrapper) = 0;
				virtual HandlerWrapper wrapHandler(std::function<void()>) = 0;

			protected:
				virtual HandlerContext<ResourceType> createHandlerContext() = 0;
				virtual HandlerWrapper               wrapHandler(std::function<void()>, bool) = 0;
		};
	}
}
