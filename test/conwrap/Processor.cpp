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

#include "Mocks.hpp"


TEST(Processor, getResource1)
{
	{
		conwrap::ProcessorMock processor;

		EXPECT_TRUE(processor.getResource() != nullptr);
	}

	{
		auto dummyPtr    = std::make_unique<Dummy>();
		auto dummyRawPtr = dummyPtr.get();
		conwrap::ProcessorMock processor(std::move(dummyPtr));

		EXPECT_EQ(dummyRawPtr, processor.getResource());
	}
}


TEST(Processor, Process1)
{
	std::atomic<bool> called;
	auto dummyPtr          = std::make_unique<Dummy>();
	auto dummyRawPtr       = dummyPtr.get();
	auto dummyResultRawPtr = dummyRawPtr;
	conwrap::ProcessorMock     processor(std::move(dummyPtr));
	conwrap::Processor<Dummy>* processorResultPtr;

	called             = false;
	dummyResultRawPtr  = nullptr;
	processorResultPtr = nullptr;
	processor.process([&](auto handlerContext)
	{
		called             = true;
		dummyResultRawPtr  = handlerContext.getResource();
		processorResultPtr = handlerContext.getProcessor();
	});

	EXPECT_TRUE(called);
	EXPECT_EQ(dummyRawPtr, dummyResultRawPtr);
	EXPECT_EQ(&processor,  processorResultPtr);
}


TEST(Processor, Process2)
{
	std::atomic<bool> called;
	auto dummyPtr          = std::make_unique<Dummy>();
	auto dummyRawPtr       = dummyPtr.get();
	auto dummyResultRawPtr = dummyRawPtr;
	conwrap::ProcessorMock     processor(std::move(dummyPtr));
	conwrap::Processor<Dummy>* processorResultPtr;

	called             = false;
	dummyResultRawPtr  = nullptr;
	processorResultPtr = nullptr;
	processor.process([&](auto handlerContext)
	{
		handlerContext.getProcessor()->process([&](auto handlerContext)
		{
			called             = true;
			dummyResultRawPtr  = handlerContext.getResource();
			processorResultPtr = handlerContext.getProcessor();
		});
	});

	EXPECT_TRUE(called);
	EXPECT_EQ(dummyRawPtr, dummyResultRawPtr);
	EXPECT_EQ(&processor,  processorResultPtr);
}


TEST(Processor, Process3)
{
	std::atomic<int>       val;
	conwrap::ProcessorMock processor;

	val = 123;
	auto syncCall = processor.process([&](auto) -> int
	{
		return val;
	});
	EXPECT_EQ(val, syncCall.get());
}
