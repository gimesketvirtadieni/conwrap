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
	// this is just a 'big-enough' counter
	// assuming increment rate is 1M per sec, it will expire in ~584942 years!
	// it can be hugely increased by implementing highValue along with lowValue, but it this case a dedicate lock must be used by generator
	class Epoch
	{
		public:
			Epoch()
			: lowValue(0) {}

			explicit Epoch(unsigned long long v)
			: lowValue(v) {}

		    inline friend bool operator<=(const Epoch& l, const Epoch& r)
		    {
		        return std::tie(l.lowValue) <= std::tie(r.lowValue);
		    }

		    inline auto getLowValue()
		    {
		        return lowValue;
		    }

		private:
		    unsigned long long lowValue;
	};
}
