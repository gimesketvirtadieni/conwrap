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
#include <conwrap/Processor.hpp>
#include <conwrap/ProcessorAsioImpl.hpp>
#include <conwrap/ProcessorAsioProxy.hpp>
#include <conwrap/ProcessorQueue.hpp>
#include <conwrap/TaskResult.hpp>
#include <conwrap/TaskWrapped.hpp>
#include <functional>
#include <memory>
#include <type_traits>


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
			: processorImplPtr(std::make_shared<internal::ProcessorAsioImpl<ResourceType>>(std::move(resource)))
			, processorProxyPtr(std::unique_ptr<ProcessorAsioProxy<ResourceType>>(new ProcessorAsioProxy<ResourceType>(processorImplPtr)))
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

			virtual ~ProcessorAsio()
			{
				processorImplPtr->stop();
			}

			asio::io_service* getDispatcher()
			{
				return processorImplPtr->getDispatcher();
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

			virtual void post(TaskWrapped task) override
			{
				processorImplPtr->post(std::move(task));
			}

			virtual TaskWrapped wrap(std::function<void()> task)
			{
				return std::move(wrap(std::move(task), false));
			}

		protected:
			template <typename T, typename = void>
			struct hasSetProcessor : std::false_type {};

			template <typename T>
			struct hasSetProcessor<T, decltype(std::declval<T>().setProcessor(std::declval<conwrap::ProcessorAsio<ResourceType>*>()))> : std::true_type {};

			template <typename T, typename = void>
			struct hasSetProcessorProxy : std::false_type {};

			template <typename T>
			struct hasSetProcessorProxy<T, decltype(std::declval<T>().setProcessorProxy((conwrap::ProcessorAsioProxy<ResourceType>*)nullptr))> : std::true_type {};

			virtual Processor<ResourceType>* getProcessor() override
			{
				return this;
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

			virtual TaskWrapped wrap(std::function<void()> task, bool proxy) override
			{
				return std::move(processorImplPtr->wrap(std::move(task), proxy));
			}

		private:
			std::shared_ptr<internal::ProcessorAsioImpl<ResourceType>> processorImplPtr;
			std::unique_ptr<ProcessorAsioProxy<ResourceType>>          processorProxyPtr;
	};
}
