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
#include <conwrap/HandlerContext.hpp>


namespace conwrap
{
	template <typename ResourceType, typename ResultType>
	class ContinuationContext : public HandlerContext<ResourceType>
	{
		public:
			ContinuationContext(ProcessorProxy<ResourceType>* p, std::shared_future<ResultType> r)
			: HandlerContext<ResourceType>(p)
			, result(r) {}

			virtual ResultType getResult()
			{
				return result.get();
			}

		private:
			std::shared_future<ResultType> result;
	};
}
