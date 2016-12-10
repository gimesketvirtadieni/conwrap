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
#include <conwrap/Processor.hpp>
#include <conwrap/ProcessorProxy.hpp>
#include <conwrap/ProcessorQueue.hpp>
#include <conwrap/ProcessorAsio.hpp>
#include <conwrap/TaskResult.hpp>
#include <conwrap/TaskResultProxy.hpp>
#include <conwrap/TaskWrapped.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>


namespace conwrap
{
	class ProcessorMock;
	class ProcessorMockProxy;
}


struct Dummy {
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
				virtual Processor<Dummy>* getProcessor() override
				{
					return processorPtr;
				}

				virtual ProcessorProxy<Dummy>* getProcessorProxy() override
				{
					return processorProxyPtr;
				}

				virtual void post(TaskWrapped handlerWrapper) override
				{
					handlerWrapper();
				}

				inline void setProcessor(Processor<Dummy>* p)
				{
					processorPtr = p;
				}

				inline void setProcessorProxy(ProcessorProxy<Dummy>* pp)
				{
					processorProxyPtr = pp;
				}

				virtual conwrap::TaskWrapped wrap(std::function<void()> handler) override
				{
					return wrap(handler, false);
				}

				virtual conwrap::TaskWrapped wrap(std::function<void()> handler, bool proxy) override
				{
					return conwrap::TaskWrapped(handler, proxy, 0);
				}

			private:
				std::unique_ptr<Dummy> resourcePtr;
				Processor<Dummy>*      processorPtr;
				ProcessorProxy<Dummy>* processorProxyPtr;
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

			virtual void post(TaskWrapped handlerWrapper) override
			{
				handlerWrapper();
			}

			virtual TaskWrapped wrap(std::function<void()> handler) override
			{
				return wrap(handler, false);
			}

		protected:
			virtual Processor<Dummy>* getProcessor() override
			{
				return processorImplPtr->getProcessor();
			}

			virtual ProcessorProxy<Dummy>* getProcessorProxy() override
			{
				return this;
			}

			virtual TaskWrapped wrap(std::function<void()> handler, bool proxy) override
			{
				return TaskWrapped(handler, proxy, 0);
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
				processorImplPtr->getResource()->setProcessor(this);
				processorImplPtr->getResource()->setProcessorProxy(processorProxyPtr.get());
			}

			virtual Dummy* getResource() override
			{
				return processorImplPtr->getResource();
			}

			virtual void flush() override {}

			virtual void post(TaskWrapped handlerWrapper) override
			{
				handlerWrapper();
			}

			virtual TaskWrapped wrap(std::function<void()> handler) override
			{
				return wrap(handler, false);
			}

		protected:
			virtual Processor<Dummy>* getProcessor() override
			{
				return this;
			}

			virtual ProcessorProxy<Dummy>* getProcessorProxy() override
			{
				return processorProxyPtr.get();
			}

			virtual TaskWrapped wrap(std::function<void()> handler, bool proxy) override
			{
				return TaskWrapped(handler, proxy, 0);
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
