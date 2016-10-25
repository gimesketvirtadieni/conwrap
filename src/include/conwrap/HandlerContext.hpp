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

#include <conwrap/TaskProxy.hpp>
#include <functional>


namespace conwrap
{
	// forward declaration
	template <typename ResourceType, template<typename ResourceType> class ResultType>
	class Processor;
	template <typename ResourceType, template<typename ResourceType> class ResultType>
	class ProcessorProxy;

	template <typename ResourceType>
	class HandlerContext
	{
		public:
			HandlerContext(ResourceType* r = nullptr, ProcessorProxy<ResourceType, TaskProxy>* p = nullptr)
			: resourcePtr(r)
			, processorProxyPtr(p) {}

			virtual ~HandlerContext() {}

			inline ProcessorProxy<ResourceType, TaskProxy>* getProcessorProxy()
			{
				return processorProxyPtr;
			}

			inline ResourceType* getResource()
			{
				return resourcePtr;
			}

		private:
			ResourceType*                            resourcePtr;
			ProcessorProxy<ResourceType, TaskProxy>* processorProxyPtr;
	};
}
