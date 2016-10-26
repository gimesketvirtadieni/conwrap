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
	// forward declaration
	template <typename ResourceType, template <typename ResourceType, typename ResultType> class TaskType>
	class ProcessorProxy;

	template <typename ResourceType, typename ResultType>
	class TaskProxy
	{
		public:
			TaskProxy(ProcessorProxy<ResourceType, TaskProxy>* p, std::shared_future<ResultType> f)
			: processorProxyPtr(p)
			, result(f) {}

			TaskProxy(const TaskProxy& rhs)
			{
				processorProxyPtr = rhs.processorProxyPtr;
				result            = rhs.result;
			}

			virtual ~TaskProxy() {}

			TaskProxy& operator= (const TaskProxy& rhs)
			{
				if (&rhs != this)
				{
					processorProxyPtr = rhs.processorProxyPtr;
					result            = rhs.result;
				}
				return *this;
			}

		private:
			ProcessorProxy<ResourceType, TaskProxy>* processorProxyPtr;
			std::shared_future<ResultType>           result;
	};
}
