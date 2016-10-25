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
#include <conwrap/HandlerWrapper.hpp>
#include <conwrap/Processor.hpp>
#include <conwrap/ProcessorQueueImpl.hpp>
#include <conwrap/ProcessorQueueProxy.hpp>
#include <conwrap/Task.hpp>
#include <memory>


namespace conwrap
{
	// forward declaration
	namespace internal
	{
		template <typename ResourceType>
		class ProcessorAsioImpl;
	}

	template <typename ResourceType>
	class ProcessorQueue : public Processor<ResourceType, Task>
	{
		// friend declaration
		template <typename> friend class internal::ProcessorAsioImpl;

		public:
			template <typename... Args>
			ProcessorQueue(Args... args)
			: ProcessorQueue(std::move(std::make_unique<ResourceType>(std::forward<Args>(args)...))) {}

			ProcessorQueue(std::unique_ptr<ResourceType> resource)
			: processorImplPtr(std::make_shared<internal::ProcessorQueueImpl<ResourceType>>(std::move(resource)))
			, processorProxyPtr(std::unique_ptr<ProcessorQueueProxy<ResourceType>>(new ProcessorQueueProxy<ResourceType>(processorImplPtr)))
			{
				// TODO: implemet compile-time reflection to make this invocation optional
				processorImplPtr->getResource()->setProcessor(this);
				processorImplPtr->setProcessorProxy(processorProxyPtr.get());
				processorImplPtr->start();
			}

			virtual ~ProcessorQueue()
			{
				processorImplPtr->stop();
			}

			virtual ResourceType* getResource() override
			{
				return processorImplPtr->getResource();
			}

			virtual void flush() override
			{
				processorImplPtr->flush();
			}

			virtual void post(HandlerWrapper handlerWrapper) override
			{
				processorImplPtr->post(handlerWrapper);
			}

			virtual HandlerWrapper wrapHandler(std::function<void()> handler)
			{
				return wrapHandler(handler, false);
			}

		protected:
			virtual HandlerContext<ResourceType> createContext() override
			{
				return processorImplPtr->createContext();
			}

			virtual HandlerWrapper wrapHandler(std::function<void()> handler, bool proxy) override
			{
				return processorImplPtr->wrapHandler(handler, proxy);
			}

		private:
			std::shared_ptr<internal::ProcessorQueueImpl<ResourceType>> processorImplPtr;
			std::unique_ptr<ProcessorQueueProxy<ResourceType>>          processorProxyPtr;
	};
}
