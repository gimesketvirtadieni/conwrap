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

#include <conwrap/Processor.hpp>
#include <conwrap/ProcessorQueueImpl.hpp>
#include <conwrap/ProcessorQueueProxy.hpp>
#include <conwrap/TaskResult.hpp>
#include <conwrap/TaskWrapped.hpp>
#include <memory>
#include <type_traits>


namespace conwrap
{
	// forward declaration
	namespace internal
	{
		template <typename ResourceType>
		class ProcessorAsioImpl;
	}
	template <typename ResourceType>
	class ProcessorQueueProxy;

	template <typename ResourceType>
	class ProcessorQueue : public Processor<ResourceType>
	{
		// friend declaration
		template <typename> friend class internal::ProcessorAsioImpl;

		public:
			template <typename... Args>
			ProcessorQueue(Args... args)
			: ProcessorQueue(std::move(std::make_unique<ResourceType>(std::forward<Args>(args)...))) {}

            ProcessorQueue(std::unique_ptr<ResourceType> resource)
			: processorImplPtr(std::make_shared<internal::ProcessorQueueImpl<ResourceType>>(std::move(resource)))
			, processorProxyPtr(std::unique_ptr<ProcessorQueueProxy<ResourceType>>(new ProcessorQueueProxy<ResourceType>(processorImplPtr)))
			{
				// weiring processors
				processorImplPtr->setProcessor(this);
				processorImplPtr->setProcessorProxy(processorProxyPtr.get());

				// this is a 'wooddoo' compile-time dependancy injection inspied by https://jguegant.github.io/blogs/tech/sfinae-introduction.html
				setProcessor(processorImplPtr->getResource());
				setProcessorProxy(processorImplPtr->getResource());

			    // starting processing
				processorImplPtr->start();
			}

			virtual ~ProcessorQueue()
			{
				processorImplPtr->stop();
			}

			virtual ProcessorProxy<ResourceType>* getProcessorProxy() override
			{
				return processorProxyPtr.get();
			}

			virtual ResourceType* getResource() override
			{
				return processorImplPtr->getResource();
			}

			virtual void flush() override
			{
				processorImplPtr->flush();
			}

		protected:
			template <typename T, typename = void>
			struct hasSetProcessor : std::false_type {};

			template <typename T>
			struct hasSetProcessor<T, decltype(std::declval<T>().setProcessor(std::declval<conwrap::ProcessorQueue<ResourceType>*>()))> : std::true_type {};

			template <typename T, typename = void>
			struct hasSetProcessorProxy : std::false_type {};

			template <typename T>
			struct hasSetProcessorProxy<T, decltype(std::declval<T>().setProcessorProxy((conwrap::ProcessorQueueProxy<ResourceType>*)nullptr))> : std::true_type {};

			virtual Processor<ResourceType>* getProcessor() override
			{
				return this;
			}

			virtual void post(TaskWrapped task) override
			{
				processorImplPtr->post(std::move(task));
			}

			template <typename T>
			typename std::enable_if<hasSetProcessor<T>::value, void>::type setProcessor(T* obj)
			{
			    obj->setProcessor(this);
			}

			template <typename T>
			typename std::enable_if<!hasSetProcessor<T>::value, void>::type setProcessor(T* obj) {}

			template <typename T>
			typename std::enable_if<hasSetProcessorProxy<T>::value, void>::type setProcessorProxy(T* obj)
			{
			    obj->setProcessorProxy(processorProxyPtr.get());
			}

			template <typename T>
			typename std::enable_if<!hasSetProcessorProxy<T>::value, void>::type setProcessorProxy(T* obj) {}

			virtual TaskWrapped wrap(std::function<void()> task) override
			{
				return std::move(wrap(std::move(task), false));
			}

			virtual TaskWrapped wrap(std::function<void()> task, bool proxy) override
			{
				return std::move(processorImplPtr->wrap(std::move(task), proxy));
			}

		private:
			std::shared_ptr<internal::ProcessorQueueImpl<ResourceType>> processorImplPtr;
			std::unique_ptr<ProcessorQueueProxy<ResourceType>>          processorProxyPtr;
	};
}
