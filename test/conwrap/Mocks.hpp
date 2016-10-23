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
#include <conwrap/ProcessorBase.hpp>
#include <conwrap/ProcessorProxy.hpp>
#include <conwrap/ProcessorQueue.hpp>
#include <conwrap/ProcessorAsio.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>


struct Dummy {
	Dummy() {}

	virtual ~Dummy() {}

	void setProcessor(conwrap::Processor<Dummy>* p)
	{
		processorPtr = p;
	}

	conwrap::Processor<Dummy>* processorPtr;
};


namespace conwrap
{
	namespace internal
	{
		class ProcessorMockBase : public ProcessorBase<Dummy>
		{
			public:
				ProcessorMockBase()
				: resourcePtr(std::make_unique<Dummy>()) {}

				ProcessorMockBase(std::unique_ptr<Dummy> r)
				: resourcePtr(std::move(r)) {}

				virtual HandlerContext<Dummy> createHandlerContext() override
				{
					return HandlerContext<Dummy> (getResource(), processorProxyPtr);
				}

				virtual Dummy* getResource() override
				{
					return resourcePtr.get();
				}

				virtual void post(HandlerWrapper handlerWrapper) override
				{
					handlerWrapper();
				}

				inline void setProcessorProxy(ProcessorProxy<Dummy>* p)
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
				std::unique_ptr<Dummy> resourcePtr;
				ProcessorProxy<Dummy>* processorProxyPtr;
		};
	}

	class ProcessorMockProxy : public ProcessorProxy<Dummy>
	{
		public:
			ProcessorMockProxy(std::shared_ptr<internal::ProcessorMockBase> p)
			: processorBasePtr(p) {}

			virtual Dummy* getResource() override
			{
				return processorBasePtr->getResource();
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
			virtual HandlerContext<Dummy> createHandlerContext() override
			{
				return HandlerContext<Dummy> (getResource(), this);
			}

			virtual HandlerWrapper wrapHandler(std::function<void()> handler, bool proxy) override
			{
				return HandlerWrapper(handler, proxy);
			}

		private:
			std::shared_ptr<internal::ProcessorMockBase> processorBasePtr;
	};

	class ProcessorMock : public Processor<Dummy>
	{
		public:
			ProcessorMock()
			: ProcessorMock(std::move(std::make_unique<Dummy>())) {}

			ProcessorMock(std::unique_ptr<Dummy> r)
			: processorBasePtr(std::make_shared<internal::ProcessorMockBase>(std::move(r)))
			, processorProxyPtr(std::unique_ptr<ProcessorMockProxy>(new ProcessorMockProxy(processorBasePtr)))
			{
				processorBasePtr->getResource()->setProcessor(this);
				processorBasePtr->setProcessorProxy(processorProxyPtr.get());
			}

			inline ProcessorMockProxy* getProcessorProxy()
			{
				return processorProxyPtr.get();
			}

			virtual Dummy* getResource() override
			{
				return processorBasePtr->getResource();
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
			virtual HandlerContext<Dummy> createHandlerContext() override
			{
				return processorBasePtr->createHandlerContext();
			}

			virtual HandlerWrapper wrapHandler(std::function<void()> handler, bool proxy) override
			{
				return HandlerWrapper(handler, proxy);
			}

		private:
			std::shared_ptr<internal::ProcessorMockBase> processorBasePtr;
			std::unique_ptr<ProcessorMockProxy>          processorProxyPtr;
	};
}


// googletest fixtures
class ProcessorCommon : public ::testing::TestWithParam<std::shared_ptr<conwrap::Processor<Dummy>>>
{
};
