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

#include <conwrap/Context.hpp>
#include <conwrap/HandlerWrapper.hpp>
#include <conwrap/Processor.hpp>
#include <conwrap/ProcessorProxy.hpp>
#include <conwrap/ProcessorQueue.hpp>
#include <conwrap/ProcessorAsio.hpp>
#include <conwrap/Provider.hpp>
#include <conwrap/TaskResult.hpp>
#include <conwrap/TaskResultProxy.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>


namespace conwrap
{
	class ProcessorMock;
	class ProcessorMockProxy;
}


struct Dummy {
	Dummy() {}

	virtual ~Dummy() {}

	void setProcessor(conwrap::Processor<Dummy>* p)
	{
		std::cout << "setProcessor called\n\r";
		processorPtr = p;
	}

	void setProcessorProxy(conwrap::ProcessorProxy<Dummy>* p)
	{
		std::cout << "setProcessorProxy called\n\r";
		processorProxyPtr = p;
	}

	conwrap::Processor<Dummy>*      processorPtr;
	conwrap::ProcessorProxy<Dummy>* processorProxyPtr;

	MOCK_METHOD0(method1, void());
	MOCK_METHOD0(method2, void());
};


namespace conwrap
{
	namespace internal
	{
		class ProcessorMockImpl : public Processor<Dummy>
		{
			// friend declaration
			friend class conwrap::ProcessorMock;
			friend class conwrap::ProcessorMockProxy;

			public:
				ProcessorMockImpl()
				: resourcePtr(std::make_unique<Dummy>()) {}

				ProcessorMockImpl(std::unique_ptr<Dummy> r)
				: resourcePtr(std::move(r)) {}

				virtual void flush() override {}

				virtual Dummy* getResource() override
				{
					return resourcePtr.get();
				}

			protected:
				virtual Provider<Dummy>* getProvider() override
				{
					return providerPtr.get();
				}

				inline Provider<Dummy>* getProviderProxy()
				{
					return providerProxyPtr.get();
				}

				virtual void post(HandlerWrapper handlerWrapper) override
				{
					handlerWrapper();
				}

				inline void setProvider(Provider<Dummy> t)
				{
					providerPtr = std::make_unique<Provider<Dummy>>(t);
				}

				inline void setProviderProxy(Provider<Dummy> t)
				{
					providerProxyPtr = std::make_unique<Provider<Dummy>>(t);
				}

				virtual conwrap::HandlerWrapper wrapHandler(std::function<void()> handler) override
				{
					return wrapHandler(handler, false);
				}

				virtual conwrap::HandlerWrapper wrapHandler(std::function<void()> handler, bool proxy) override
				{
					return conwrap::HandlerWrapper(handler, proxy, 0);
				}

			private:
				std::unique_ptr<Dummy>           resourcePtr;
				std::unique_ptr<Provider<Dummy>> providerPtr;
				std::unique_ptr<Provider<Dummy>> providerProxyPtr;
		};
	}

	class ProcessorMockProxy : public ProcessorProxy<Dummy>
	{
		public:
			ProcessorMockProxy(std::shared_ptr<internal::ProcessorMockImpl> p)
			: processorImplPtr(p) {}

			virtual Dummy* getResource() override
			{
				return processorImplPtr->getResource();
			}

			virtual void post(HandlerWrapper handlerWrapper) override
			{
				handlerWrapper();
			}

			virtual HandlerWrapper wrapHandler(std::function<void()> handler) override
			{
				return wrapHandler(handler, false);
			}

		protected:
			virtual Provider<Dummy>* getProvider() override
			{
				return processorImplPtr->getProviderProxy();
			}

			virtual HandlerWrapper wrapHandler(std::function<void()> handler, bool proxy) override
			{
				return HandlerWrapper(handler, proxy, 0);
			}

		private:
			std::shared_ptr<internal::ProcessorMockImpl> processorImplPtr;
	};

	class ProcessorMock : public Processor<Dummy>
	{
		public:
			ProcessorMock()
			: ProcessorMock(std::move(std::make_unique<Dummy>())) {}

			ProcessorMock(std::unique_ptr<Dummy> r)
			: processorImplPtr(std::make_shared<internal::ProcessorMockImpl>(std::move(r)))
			, processorProxyPtr(std::unique_ptr<ProcessorMockProxy>(new ProcessorMockProxy(processorImplPtr)))
			{
				processorImplPtr->setProvider(Provider<Dummy>(this, processorProxyPtr.get()));
				processorImplPtr->setProviderProxy(Provider<Dummy>(this, processorProxyPtr.get()));
				processorImplPtr->getResource()->setProcessor(this);
				processorImplPtr->getResource()->setProcessorProxy(processorProxyPtr.get());
			}

			virtual Dummy* getResource() override
			{
				return processorImplPtr->getResource();
			}

			virtual void flush() override {}

			virtual void post(HandlerWrapper handlerWrapper) override
			{
				handlerWrapper();
			}

			virtual HandlerWrapper wrapHandler(std::function<void()> handler) override
			{
				return wrapHandler(handler, false);
			}

		protected:
			virtual Provider<Dummy>* getProvider() override
			{
				return processorImplPtr->getProvider();
			}

			virtual HandlerWrapper wrapHandler(std::function<void()> handler, bool proxy) override
			{
				return HandlerWrapper(handler, proxy, 0);
			}

		private:
			std::shared_ptr<internal::ProcessorMockImpl> processorImplPtr;
			std::unique_ptr<ProcessorMockProxy>          processorProxyPtr;
	};
}


// googletest fixtures
class ProcessorCommon : public ::testing::TestWithParam<std::shared_ptr<conwrap::Processor<Dummy>>>
{
};
