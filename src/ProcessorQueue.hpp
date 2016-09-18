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
#include "HandlerContext.hpp"
#include "ProcessorQueueImpl.hpp"


template <typename ResourceType>
class ProcessorQueue : public Processor<ResourceType>
{
	public:
		template <typename... Args>
		ProcessorQueue(Args... args)
		: processorImplPtr(std::make_shared<ProcessorQueueImpl<ResourceType>>(std::move(std::make_unique<ResourceType>(this, std::forward<Args>(args)...))))
		, processorProxyPtr(std::unique_ptr<ProcessorQueue<ResourceType>>(new ProcessorQueue<ResourceType>(processorImplPtr)))
		, proxy(false)
		{
			processorImplPtr->start();
		}

		ProcessorQueue(std::unique_ptr<ResourceType> resource)
		: processorImplPtr(std::make_shared<ProcessorQueueImpl<ResourceType>>(std::move(resource)))
		, processorProxyPtr(std::unique_ptr<ProcessorQueue<ResourceType>>(new ProcessorQueue<ResourceType>(processorImplPtr)))
		, proxy(false)
		{
			processorImplPtr->start();
		}

		virtual ~ProcessorQueue()
		{
			if (!proxy)
			{
				processorImplPtr->stop();
			}
		}

		virtual ResourceType* getResource() override
		{
			return processorImplPtr->getResource();
		}

		virtual void flush() override
		{
			processorImplPtr->flush();
		}

		virtual HandlerWrapper wrapHandler(std::function<void()> handler) override
		{
			return processorImplPtr->wrapHandler(handler);
		}

	protected:
		ProcessorQueue(std::shared_ptr<ProcessorQueueImpl<ResourceType>> processorImplPtr)
		: processorImplPtr(processorImplPtr)
		, proxy(true) {}

		virtual void post(std::function<void()> handler) override
		{
			processorImplPtr->post(handler);
		}

	private:
		std::shared_ptr<ProcessorQueueImpl<ResourceType>> processorImplPtr;
		std::unique_ptr<ProcessorQueue<ResourceType>>     processorProxyPtr;
		bool                                              proxy;
};
