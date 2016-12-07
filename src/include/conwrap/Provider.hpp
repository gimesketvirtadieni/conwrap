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
	template <typename ResourceType>
	class Processor;
	template <typename ResourceType>
	class ProcessorProxy;

	template <typename ResourceType>
	class Provider
	{
		public:
			explicit Provider(Processor<ResourceType>* p, ProcessorProxy<ResourceType>* pp)
			: processorPtr(p)
			, processorProxyPtr(pp) {}

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
