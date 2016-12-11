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

#include <conwrap/Epoch.hpp>
#include <functional>


namespace conwrap
{
	class TaskWrapped
	{
		public:
			explicit TaskWrapped(std::function<void()> t, bool p, conwrap::Epoch e)
			: task(std::move(t))
			, proxy(p)
			, epoch(e) {}

			// copy contructor is required to comply with Asio handler
			TaskWrapped(const TaskWrapped& c)
			: task(std::move(c.task))
			, proxy(c.proxy)
			, epoch(c.epoch) {}

			TaskWrapped(TaskWrapped&& c)
			: task(std::move(c.task))
			, proxy(c.proxy)
			, epoch(c.epoch) {}

			inline auto getEpoch()
			{
				return epoch;
			}

			inline auto getProxy()
			{
				return proxy;
			}

			inline auto operator()()
			{
				task();
			}

		private:
			std::function<void()> task;
			bool                  proxy;
			const Epoch           epoch;
	};
}
