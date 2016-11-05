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
	// assuming increment rate is 1M per sec, it will expire in ~58.5 years!
	// it can be hugely increased by implementing highValue along with lowValue
	class Epoch
	{
		public:
			Epoch(unsigned long long v)
			: lowValue(v) {}

			virtual ~Epoch() {}

			inline Epoch& operator++()
			{
				// incrementing value and returning result
				lowValue++;
				return *this;
			}

			inline Epoch operator++(int)
			{
				// saving value before post-increment
				Epoch tmp(*this);
				operator++();
				return tmp;
			}

		    inline friend bool operator<(const Epoch& l, const Epoch& r)
		    {
		        return std::tie(l.lowValue) < std::tie(r.lowValue);
		    }

		    inline auto getLowValue()
		    {
		        return lowValue;
		    }

		private:
			unsigned long long lowValue;
	};
}
