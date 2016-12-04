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

#include <conwrap/Context.hpp>
#include <conwrap/Handler.hpp>
#include <conwrap/HandlerWithContext.hpp>
#include <future>


namespace conwrap
{
	// forward declaration
	template <typename ResourceType>
	class Processor;
	template <typename ResourceType>
	class ProcessorProxy;

	template <typename ResourceType, template<typename ResourceType, typename ResultType> class TaskResultType>
	class Provider
	{
		public:
			explicit Provider(Processor<ResourceType>* p, ProcessorProxy<ResourceType>* pp)
			: processorPtr(p)
			, processorProxyPtr(pp) {}

			inline Context<ResourceType> createContext()
			{
				return Context<ResourceType>(getProcessorProxy());
			}

			// TODO: createTaskResult(...) should be refactored
			template <typename F, typename ResultType>
			auto createTaskResult(Handler<ResourceType, F, ResultType>& task) -> TaskResultType<ResourceType, ResultType>
			{
				return TaskResultType<ResourceType, ResultType>(getProcessor(), getProcessorProxy(), task.getFuture());
			}

			template <typename F, typename ResultType>
			auto createTaskResult(HandlerWithContext<ResourceType, F, ResultType>& task) -> TaskResultType<ResourceType, ResultType>
			{
				return TaskResultType<ResourceType, ResultType>(getProcessor(), getProcessorProxy(), task.getFuture());
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
