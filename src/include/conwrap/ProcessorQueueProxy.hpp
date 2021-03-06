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
#include <conwrap/ProcessorQueueImpl.hpp>
#include <conwrap/ProcessorProxy.hpp>
#include <conwrap/TaskResultProxy.hpp>
#include <conwrap/TaskWrapped.hpp>


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

			virtual ProcessorProxy<ResourceType>* getProcessorProxy() override
			{
				return this;
			}

			virtual ResourceType* getResource() override
			{
				return processorImplPtr->getResource();
			}

		protected:
			virtual Processor<ResourceType>* getProcessor() override
			{
				return processorImplPtr->getProcessor();
			}

			virtual void post(TaskWrapped task) override
			{
				processorImplPtr->post(std::move(task));
			}

			virtual TaskWrapped wrap(std::function<void()> task) override
			{
				return std::move(wrap(std::move(task), true));
			}

			virtual TaskWrapped wrap(std::function<void()> task, bool proxy) override
			{
				return std::move(processorImplPtr->wrap(std::move(task), proxy));
			}

		private:
			std::shared_ptr<internal::ProcessorQueueImpl<ResourceType>> processorImplPtr;
	};
}
