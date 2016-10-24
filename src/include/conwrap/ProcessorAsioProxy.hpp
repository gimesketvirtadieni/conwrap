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
#include <conwrap/ProcessorProxy.hpp>


namespace conwrap
{
	template <typename ResourceType>
	class ProcessorAsioProxy : public ProcessorProxy<ResourceType>
	{
		public:
			ProcessorAsioProxy(std::shared_ptr<internal::ProcessorAsioBase<ResourceType>> p)
			: processorBasePtr(p) {}

			virtual ~ProcessorAsioProxy() {}

			asio::io_service* getDispatcher()
			{
				return processorBasePtr->getDispatcher();
			}

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
			virtual HandlerContext<ResourceType> createContext() override
			{
				return processorBasePtr->createContext();
			}

			virtual HandlerWrapper wrapHandler(std::function<void()> handler, bool proxy) override
			{
				return processorBasePtr->wrapHandler(handler, proxy);
			}

		private:
			std::shared_ptr<internal::ProcessorAsioBase<ResourceType>> processorBasePtr;
	};

}
