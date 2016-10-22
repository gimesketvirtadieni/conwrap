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
#include <conwrap/Processor.hpp>
#include <conwrap/ProcessorQueueBase.hpp>


namespace conwrap
{
	// forward declaration
	template <typename ResourceType>
	class ProcessorAsio;

	template <typename ResourceType>
	class ProcessorQueue : public Processor<ResourceType>
	{
		// friend declaration
		template <typename> friend class ProcessorAsio;

		public:
			template <typename... Args>
			ProcessorQueue(Args... args)
			: processorBasePtr(std::make_shared<internal::ProcessorQueueBase<ResourceType>>(std::move(std::make_unique<ResourceType>(std::forward<Args>(args)...))))
			, processorProxyPtr(std::unique_ptr<ProcessorQueue<ResourceType>>(new ProcessorQueue<ResourceType>(processorBasePtr)))
			, proxy(false)
			{
				processorBasePtr->setProcessor(processorProxyPtr.get());
				processorBasePtr->start();
			}

			ProcessorQueue(std::unique_ptr<ResourceType> resource)
			: processorBasePtr(std::make_shared<internal::ProcessorQueueBase<ResourceType>>(std::move(resource)))
			, processorProxyPtr(std::unique_ptr<ProcessorQueue<ResourceType>>(new ProcessorQueue<ResourceType>(processorBasePtr)))
			, proxy(false)
			{
				processorBasePtr->setProcessor(processorProxyPtr.get());
				processorBasePtr->start();
			}

			virtual ~ProcessorQueue()
			{
				if (!proxy)
				{
					processorBasePtr->stop();
				}
			}

			virtual HandlerContext<ResourceType> createHandlerContext() override
			{
				return processorBasePtr->createHandlerContext();
			}

			virtual ResourceType* getResource() override
			{
				return processorBasePtr->getResource();
			}

			virtual void flush() override
			{
				// figuring out current epoch
				auto currentEpoch = this->process([=]() -> auto
				{
					return processorBasePtr->getEpoch();
				}).get();

				// waiting for any 'child' handlers to be processed
				while (processorBasePtr->childExists(currentEpoch))
				{
					// TODO: insert flush handler after the last child instead of adding at the end of the queue
					this->process([=] {}).wait();
				}
			}

			virtual void post(HandlerWrapper handlerWrapper) override
			{
				processorBasePtr->post(handlerWrapper);
			}

			virtual HandlerWrapper wrapHandler(std::function<void()> handler)
			{
				return wrapHandler(handler, proxy);
			}

		protected:
			ProcessorQueue(std::shared_ptr<internal::ProcessorQueueBase<ResourceType>> processorBasePtr)
			: processorBasePtr(processorBasePtr)
			, proxy(true) {}

			virtual HandlerWrapper wrapHandler(std::function<void()> handler, bool proxy) override
			{
				return processorBasePtr->wrapHandler(handler, proxy);
			}

		private:
			std::shared_ptr<internal::ProcessorQueueBase<ResourceType>> processorBasePtr;
			std::unique_ptr<ProcessorQueue<ResourceType>>               processorProxyPtr;
			bool                                                        proxy;
	};

}
