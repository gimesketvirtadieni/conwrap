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

#include <atomic>
#include <conwrap/Epoch.hpp>
#include <functional>


namespace conwrap
{
	class EpochGenerator
	{
		public:
			EpochGenerator()
			: lowValue(0) {}

			inline auto current()
			{
				return Epoch(lowValue.load());
			}

			inline auto generate()
			{
				return Epoch(++lowValue);
			}

		private:
		    std::atomic<decltype(Epoch().getLowValue())> lowValue;
	};
}
