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

#include <asio.hpp>
#include <conwrap/HandlerContext.hpp>
#include <conwrap/HandlerWrapper.hpp>
#include <conwrap/Processor.hpp>
#include <conwrap/ProcessorAsioImpl.hpp>
#include <conwrap/ProcessorAsioProxy.hpp>
#include <conwrap/ProcessorQueue.hpp>
#include <conwrap/Task.hpp>
#include <functional>
#include <memory>


namespace conwrap
{
	template <typename ResourceType>
	class ProcessorAsio : public Processor<ResourceType, Task>
	{
		public:
			template <typename... Args>
			ProcessorAsio(Args... args)
			: ProcessorAsio(std::move(std::make_unique<ResourceType>(std::forward<Args>(args)...))) {}

			ProcessorAsio(std::unique_ptr<ResourceType> resource)
			: processorImplPtr(std::make_shared<internal::ProcessorAsioImpl<ResourceType>>(std::move(resource)))
			, processorProxyPtr(std::unique_ptr<ProcessorAsioProxy<ResourceType>>(new ProcessorAsioProxy<ResourceType>(processorImplPtr)))
			{
				// TODO: implemet compile-time reflection to make this invocation optional
				processorImplPtr->getResource()->setProcessor(this);
				processorImplPtr->setProcessorProxy(processorProxyPtr.get());
				processorImplPtr->start();
			}

			virtual ~ProcessorAsio()
			{
				processorImplPtr->stop();
			}

			asio::io_service* getDispatcher()
			{
				return processorImplPtr->getDispatcher();
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
			std::shared_ptr<internal::ProcessorAsioImpl<ResourceType>> processorImplPtr;
			std::unique_ptr<ProcessorAsioProxy<ResourceType>>          processorProxyPtr;
	};

}
