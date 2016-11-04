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
#include <conwrap/ProcessorAsioImpl.hpp>
#include <conwrap/ProcessorProxy.hpp>
#include <conwrap/Provider.hpp>
#include <conwrap/TaskProxy.hpp>


namespace conwrap
{
	template <typename ResourceType>
	class ProcessorAsioProxy : public ProcessorProxy<ResourceType>
	{
		public:
			ProcessorAsioProxy(std::shared_ptr<internal::ProcessorAsioImpl<ResourceType>> p)
			: processorImplPtr(p) {}

			virtual ~ProcessorAsioProxy() {}

			asio::io_service* getDispatcher()
			{
				return processorImplPtr->getDispatcher();
			}

			virtual ResourceType* getResource() override
			{
				return processorImplPtr->getResource();
			}

			virtual void post(HandlerWrapper handlerWrapper) override
			{
				processorImplPtr->post(handlerWrapper);
			}

			virtual HandlerWrapper wrapHandler(std::function<void()> handler)
			{
				return wrapHandler(handler, true);
			}

		protected:
			virtual Provider<ResourceType, TaskProxy>* getProvider() override
			{
				return processorImplPtr->getProviderProxy();
			}

			virtual HandlerWrapper wrapHandler(std::function<void()> handler, bool proxy) override
			{
				return processorImplPtr->wrapHandler(handler, proxy);
			}

		private:
			std::shared_ptr<internal::ProcessorAsioImpl<ResourceType>> processorImplPtr;
	};

}
