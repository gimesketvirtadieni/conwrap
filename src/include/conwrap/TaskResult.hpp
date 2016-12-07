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
#include <conwrap/TaskResultBase.hpp>


namespace conwrap
{
	// forward declaration
	template <typename ResourceType>
	class Processor;
	template <typename ResourceType>
	class ProcessorProxy;

	template <typename ResourceType, typename ResultType>
	class TaskResult : public TaskResultBase<ResourceType, ResultType, TaskResult>
	{
		public:
			explicit TaskResult(Processor<ResourceType>* p, ProcessorProxy<ResourceType>* pp, std::shared_future<ResultType> rf)
			: TaskResultBase<ResourceType, ResultType, TaskResult>(p, pp, rf) {}

			inline ResultType get()
			{
				return this->getResultFuture().get();
			}

			inline void wait()
			{
				this->getResultFuture().wait();
			}
	};
}
