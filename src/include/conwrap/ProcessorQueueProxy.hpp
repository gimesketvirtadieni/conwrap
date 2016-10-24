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

#include <memory>
#include <conwrap/HandlerContext.hpp>
#include <conwrap/HandlerWrapper.hpp>
#include <conwrap/ProcessorQueueBase.hpp>
#include <conwrap/ProcessorProxy.hpp>


namespace conwrap
{
	// forward declaration
	template <typename ResourceType>
	class ProcessorAsio;

	template <typename ResourceType>
	class ProcessorQueueProxy : public ProcessorProxy<ResourceType>
	{
		// friend declaration
		template <typename> friend class ProcessorAsio;

		public:
			ProcessorQueueProxy(std::shared_ptr<internal::ProcessorQueueBase<ResourceType>> processorBasePtr)
			: processorBasePtr(processorBasePtr) {}

			virtual ~ProcessorQueueProxy() {}

			virtual ResourceType* getResource() override
			{
				return processorBasePtr->getResource();
			}

			virtual void post(HandlerWrapper handlerWrapper) override
			{
				processorBasePtr->post(handlerWrapper);
			}

			virtual HandlerWrapper wrapHandler(std::function<void()> handler)
			{
				return wrapHandler(handler, true);
			}

		protected:
			virtual HandlerContext<ResourceType> createHandlerContext() override
			{
				return processorBasePtr->createHandlerContext();
			}

			virtual HandlerWrapper wrapHandler(std::function<void()> handler, bool proxy) override
			{
				return processorBasePtr->wrapHandler(handler, proxy);
			}

		private:
			std::shared_ptr<internal::ProcessorQueueBase<ResourceType>> processorBasePtr;
	};

}
