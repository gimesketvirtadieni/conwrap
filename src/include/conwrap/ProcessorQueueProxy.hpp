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

#include <functional>
#include <memory>
#include <conwrap/HandlerWrapper.hpp>
#include <conwrap/ProcessorQueueImpl.hpp>
#include <conwrap/ProcessorProxy.hpp>
#include <conwrap/Provider.hpp>
#include <conwrap/TaskProxy.hpp>


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
			ProcessorQueueProxy(std::shared_ptr<internal::ProcessorQueueImpl<ResourceType>> p)
			: processorImplPtr(p) {}

			virtual ~ProcessorQueueProxy() {}

			virtual ResourceType* getResource() override
			{
				return processorImplPtr->getResource();
			}

		protected:
			virtual Provider<ResourceType, TaskProxy>* getProvider() override
			{
				return processorImplPtr->getProviderProxy();
			}

			virtual void post(HandlerWrapper handlerWrapper) override
			{
				processorImplPtr->post(handlerWrapper);
			}

			virtual HandlerWrapper wrapHandler(std::function<void()> handler) override
			{
				return wrapHandler(handler, true);
			}

			virtual HandlerWrapper wrapHandler(std::function<void()> handler, bool proxy) override
			{
				return processorImplPtr->wrapHandler(handler, proxy);
			}

		private:
			std::shared_ptr<internal::ProcessorQueueImpl<ResourceType>> processorImplPtr;
	};

}
