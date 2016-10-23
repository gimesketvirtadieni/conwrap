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


namespace conwrap
{
	template <typename ResultType>
	class Task
	{
		public:
			Task(std::shared_future<ResultType> f)
			: result(f) {}

			virtual ~Task() {}

			virtual ResultType get()
			{
				return result.get();
			}

			virtual void wait()
			{
				result.wait();
			}

			Task(const Task& other)
			{
				result = other.result;
			}

		private:
			std::shared_future<ResultType> result;
	};
}
