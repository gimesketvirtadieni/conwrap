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

#include <conwrap/HandlerContext.hpp>
#include <future>


namespace conwrap
{
	// forward declaration
	template <typename ResourceType>
	class Processor;
	template <typename ResourceType>
	class ProcessorProxy;

	template <typename ResourceType, template<typename ResourceType, typename ResultType> class TaskType>
	class Provider
	{
		public:
			Provider(Processor<ResourceType>* p, ProcessorProxy<ResourceType>* pp)
			: processorPtr(p)
			, processorProxyPtr(pp) {}

			virtual ~Provider() {}

			inline HandlerContext<ResourceType> createContext()
			{
				return HandlerContext<ResourceType>(getProcessorProxy());
			}

			template <typename ResultType>
			auto createTask(std::shared_future<ResultType> result) -> TaskType<ResourceType, ResultType>
			{
				return TaskType<ResourceType, ResultType>(getProcessor(), getProcessorProxy(), result);
			}

			inline Processor<ResourceType>* getProcessor()
			{
				return processorPtr;
			}

			inline ProcessorProxy<ResourceType>* getProcessorProxy()
			{
				return processorProxyPtr;
			}

		private:
			Processor<ResourceType>*      processorPtr;
			ProcessorProxy<ResourceType>* processorProxyPtr;
	};
}
