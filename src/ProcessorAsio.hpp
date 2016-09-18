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
#include "ProcessorAsioImpl.hpp"


template <typename ResourceType>
class ProcessorAsio : public Processor<ResourceType>
{
	public:
		template <typename... Args>
		ProcessorAsio(Args... args)
		: processorImplPtr(std::make_shared<ProcessorAsioImpl<ResourceType>>(std::move(std::make_unique<ResourceType>(this, std::forward<Args>(args)...))))
		, processorProxyPtr(std::unique_ptr<ProcessorAsio<ResourceType>>(new ProcessorAsio<ResourceType>(processorImplPtr)))
		, proxy(false)
		{
			processorImplPtr->start();
		}

		template <typename... Args>
		ProcessorAsio(std::unique_ptr<ResourceType> resource)
		: processorImplPtr(std::make_shared<ProcessorAsioImpl<ResourceType>>(std::move(resource)))
		, processorProxyPtr(std::unique_ptr<ProcessorAsio<ResourceType>>(new ProcessorAsio<ResourceType>(processorImplPtr)))
		, proxy(false)
		{
			processorImplPtr->start();
		}

		virtual ~ProcessorAsio()
		{
			if (!proxy)
			{
				processorImplPtr->stop();
			}
		}

		boost::asio::io_service* getDispatcher()
		{
			return processorImplPtr->getDispatcher();
		}

		virtual ResourceType* getResource()
		{
			return processorImplPtr->getResource();
		}

		virtual void flush()
		{
			processorImplPtr->flush();
		}

	protected:
		ProcessorAsio(std::shared_ptr<ProcessorAsioImpl<ResourceType>> processorImplPtr)
		: processorImplPtr(processorImplPtr)
		, proxy(true)
		{
		}

		virtual void post(std::function<void()> handler)
		{
			processorImplPtr->post(handler);
		}

	private:
		std::shared_ptr<ProcessorAsioImpl<ResourceType>> processorImplPtr;
		std::unique_ptr<ProcessorAsio<ResourceType>>     processorProxyPtr;
		bool                                             proxy;
};
