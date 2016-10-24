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


namespace conwrap
{
	class HandlerWrapper
	{
		public:
			HandlerWrapper(std::function<void()> h, bool p, unsigned long long e = 0)
			: handler(h)
			, proxy(p)
			, epoch(e) {}

			virtual ~HandlerWrapper() {}

			inline auto getEpoch()
			{
				return epoch;
			}

			inline auto getHandler()
			{
				return handler;
			}

			inline auto getProxy()
			{
				return proxy;
			}

			inline auto operator()()
			{
				handler();
			}

		private:
			std::function<void()> handler;
			bool                  proxy;
			unsigned long long    epoch;
	};
}
