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
#include <conwrap/Processor.hpp>
#include <conwrap/ProcessorQueue.hpp>
#include <conwrap/ProcessorAsio.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>


struct Dummy {
	Dummy() {}
	Dummy(conwrap::Processor<Dummy>* processorPtr) : processorPtr(processorPtr) {}
	virtual ~Dummy() {}

	conwrap::Processor<Dummy>* processorPtr;
};


namespace conwrap
{

	class ProcessorMock : public conwrap::Processor<Dummy>
	{
		public:
			ProcessorMock() : resourcePtr(std::make_unique<Dummy>()) {}

			ProcessorMock(std::unique_ptr<Dummy> r) : resourcePtr(std::move(r)) {}

			virtual Dummy* getResource() override
			{
				return resourcePtr.get();
			}

			virtual void flush() override {}

			virtual void post(HandlerWrapper handlerWrapper) override
			{
				handlerWrapper();
			}

			virtual conwrap::HandlerWrapper wrapHandler(std::function<void()> handler) override
			{
				return conwrap::HandlerWrapper(handler);
			}

		private:
			std::unique_ptr<Dummy> resourcePtr;
	};

}