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

#include <conwrap/HandlerContext.hpp>
#include <conwrap/HandlerWrapper.hpp>
#include <conwrap/Processor.hpp>
#include <conwrap/ProcessorProxy.hpp>
#include <conwrap/ProcessorQueue.hpp>
#include <conwrap/ProcessorAsio.hpp>
#include <conwrap/Task.hpp>
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

	void setProcessor(conwrap::Processor<Dummy, conwrap::Task>* p)
	{
		processorPtr = p;
	}

	// TODO: temporary fix
	void setProcessor(conwrap::ProcessorMockProxy* p) {}

	conwrap::Processor<Dummy, conwrap::Task>* processorPtr;
};


namespace conwrap
{
	namespace internal
	{
		class ProcessorMockImpl : public Processor<Dummy, Task>
		{
			public:
				ProcessorMockImpl()
				: resourcePtr(std::make_unique<Dummy>()) {}

				ProcessorMockImpl(std::unique_ptr<Dummy> r)
				: resourcePtr(std::move(r)) {}

				virtual HandlerContext<Dummy> createContext() override
				{
					return HandlerContext<Dummy> (getResource(), processorProxyPtr);
				}

				virtual void flush() override {}

				virtual Dummy* getResource() override
				{
					return resourcePtr.get();
				}

				virtual void post(HandlerWrapper handlerWrapper) override
				{
					handlerWrapper();
				}

				inline void setProcessorProxy(ProcessorProxy<Dummy, TaskProxy>* p)
				{
					processorProxyPtr = p;
				}

				virtual conwrap::HandlerWrapper wrapHandler(std::function<void()> handler) override
				{
					return wrapHandler(handler, false);
				}

				virtual conwrap::HandlerWrapper wrapHandler(std::function<void()> handler, bool proxy) override
				{
					return conwrap::HandlerWrapper(handler, proxy);
				}

			private:
				std::unique_ptr<Dummy>            resourcePtr;
				ProcessorProxy<Dummy, TaskProxy>* processorProxyPtr;
		};
	}

	class ProcessorMockProxy : public ProcessorProxy<Dummy, TaskProxy>
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
			virtual HandlerContext<Dummy> createContext() override
			{
				return HandlerContext<Dummy> (getResource(), this);
			}

			virtual HandlerWrapper wrapHandler(std::function<void()> handler, bool proxy) override
			{
				return HandlerWrapper(handler, proxy);
			}

		private:
			std::shared_ptr<internal::ProcessorMockImpl> processorImplPtr;
	};

	class ProcessorMock : public Processor<Dummy, conwrap::Task>
	{
		public:
			ProcessorMock()
			: ProcessorMock(std::move(std::make_unique<Dummy>())) {}

			ProcessorMock(std::unique_ptr<Dummy> r)
			: processorImplPtr(std::make_shared<internal::ProcessorMockImpl>(std::move(r)))
			, processorProxyPtr(std::unique_ptr<ProcessorMockProxy>(new ProcessorMockProxy(processorImplPtr)))
			{
				processorImplPtr->getResource()->setProcessor(this);
				processorImplPtr->setProcessorProxy(processorProxyPtr.get());
			}

			inline ProcessorMockProxy* getProcessorProxy()
			{
				return processorProxyPtr.get();
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
			virtual HandlerContext<Dummy> createContext() override
			{
				return processorImplPtr->createContext();
			}

			virtual HandlerWrapper wrapHandler(std::function<void()> handler, bool proxy) override
			{
				return HandlerWrapper(handler, proxy);
			}

		private:
			std::shared_ptr<internal::ProcessorMockImpl> processorImplPtr;
			std::unique_ptr<ProcessorMockProxy>          processorProxyPtr;
	};
}


// googletest fixtures
class ProcessorCommon : public ::testing::TestWithParam<std::shared_ptr<conwrap::Processor<Dummy, conwrap::Task>>>
{
};
