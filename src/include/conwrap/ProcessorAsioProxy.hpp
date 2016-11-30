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
#include <conwrap/HandlerWrapper.hpp>
#include <conwrap/ProcessorAsioImpl.hpp>
#include <conwrap/ProcessorProxy.hpp>
#include <conwrap/Provider.hpp>
#include <conwrap/TaskResultProxy.hpp>


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
				processorImplPtr->post(std::move(handlerWrapper));
			}

			virtual HandlerWrapper wrapHandler(std::function<void()> handler)
			{
				return std::move(wrapHandler(std::move(handler), true));
			}

		protected:
			virtual Provider<ResourceType, TaskResultProxy>* getProvider() override
			{
				return processorImplPtr->getProviderProxy();
			}

			virtual HandlerWrapper wrapHandler(std::function<void()> handler, bool proxy) override
			{
				return std::move(processorImplPtr->wrapHandler(std::move(handler), proxy));
			}

		private:
			std::shared_ptr<internal::ProcessorAsioImpl<ResourceType>> processorImplPtr;
	};
}
