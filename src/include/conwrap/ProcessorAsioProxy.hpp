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
#include <conwrap/ProcessorAsioImpl.hpp>
#include <conwrap/ProcessorProxy.hpp>
#include <conwrap/TaskResultProxy.hpp>
#include <conwrap/TaskWrapped.hpp>


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

			virtual void post(TaskWrapped task) override
			{
				processorImplPtr->post(std::move(task));
			}

			virtual TaskWrapped wrap(std::function<void()> task)
			{
				return std::move(wrap(std::move(task), true));
			}

		protected:
			virtual Processor<ResourceType>* getProcessor() override
			{
				return processorImplPtr->getProcessor();
			}

			virtual ProcessorProxy<ResourceType>* getProcessorProxy() override
			{
				return this;
			}

			virtual TaskWrapped wrap(std::function<void()> task, bool proxy) override
			{
				return std::move(processorImplPtr->wrap(std::move(task), proxy));
			}

		private:
			std::shared_ptr<internal::ProcessorAsioImpl<ResourceType>> processorImplPtr;
	};
}
