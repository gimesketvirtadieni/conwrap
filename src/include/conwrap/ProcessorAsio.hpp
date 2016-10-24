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
#include <functional>
#include <memory>
#include <conwrap/HandlerContext.hpp>
#include <conwrap/HandlerWrapper.hpp>
#include <conwrap/Processor.hpp>
#include <conwrap/ProcessorAsioBase.hpp>
#include <conwrap/ProcessorAsioProxy.hpp>
#include <conwrap/ProcessorQueue.hpp>


namespace conwrap
{
	template <typename ResourceType>
	class ProcessorAsio : public Processor<ResourceType>
	{
		public:
			template <typename... Args>
			ProcessorAsio(Args... args)
			: ProcessorAsio(std::move(std::make_unique<ResourceType>(std::forward<Args>(args)...))) {}

			ProcessorAsio(std::unique_ptr<ResourceType> resource)
			: processorBasePtr(std::make_shared<internal::ProcessorAsioBase<ResourceType>>(std::move(resource)))
			, processorProxyPtr(std::unique_ptr<ProcessorAsioProxy<ResourceType>>(new ProcessorAsioProxy<ResourceType>(processorBasePtr)))
			{
				// TODO: implemet compile-time reflection to make this invocation optional
				processorBasePtr->getResource()->setProcessor(this);
				processorBasePtr->setProcessorProxy(processorProxyPtr.get());
				processorBasePtr->start();
			}

			virtual ~ProcessorAsio()
			{
				processorBasePtr->stop();
			}

			asio::io_service* getDispatcher()
			{
				return processorBasePtr->getDispatcher();
			}

			virtual ResourceType* getResource() override
			{
				return processorBasePtr->getResource();
			}

			virtual void flush() override
			{
				// restarting dispatcher which will make sure all handlers are processed
				processorBasePtr->restartDispatcher();

				// making sure the main loop started after the restart
				this->process([] {}).wait();
			}

			virtual void post(HandlerWrapper handlerWrapper) override
			{
				processorBasePtr->post(handlerWrapper);
			}

			virtual HandlerWrapper wrapHandler(std::function<void()> handler)
			{
				return wrapHandler(handler, false);
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
			std::shared_ptr<internal::ProcessorAsioBase<ResourceType>> processorBasePtr;
			std::unique_ptr<ProcessorAsioProxy<ResourceType>>          processorProxyPtr;
	};

}
