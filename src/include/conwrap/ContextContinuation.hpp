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
#include <conwrap/Context.hpp>


namespace conwrap
{
	template <typename ResourceType, typename ResultType>
	class ContextContinuation : public Context<ResourceType>
	{
		public:
			explicit ContextContinuation(ProcessorProxy<ResourceType>* p, std::shared_future<ResultType> r)
			: Context<ResourceType>(p)
			, resultFuture(r) {}

			inline ResultType getResult()
			{
				return resultFuture.get();
			}

		private:
			std::shared_future<ResultType> resultFuture;
	};
}
