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

#include <chrono>
#include "Mocks.hpp"


TEST(ProcessorQueue, Getters1)
{
	{
		ProcessorQueue<Dummy> processor;

		EXPECT_TRUE(processor.getResource() != nullptr);
		EXPECT_EQ(&processor, processor.getResource()->processorPtr);
	}

	{
		auto                  dummyPtr    = std::make_unique<Dummy>();
		auto                  dummyRawPtr = dummyPtr.get();
		ProcessorQueue<Dummy> processor(std::move(dummyPtr));

		EXPECT_EQ(dummyRawPtr, processor.getResource());
		EXPECT_EQ(&processor, processor.getResource()->processorPtr);
	}
}


TEST(ProcessorQueue, Destructor1) {
	std::atomic<bool>     called;

	// processor's scope used to activate destructor before exiting test case
	{
		ProcessorQueue<Dummy> processor;

		called = false;
		processor.process([&](auto)
		{
			// simulating some action
			std::chrono::milliseconds wait{10};
			std::this_thread::sleep_for(wait);

			called = true;
		});
		EXPECT_FALSE(called);
	}
	EXPECT_TRUE(called);
}


TEST(ProcessorQueue, Destructor2) {
	std::atomic<bool>     called;

	// processor's scope used to activate destructor before exiting test case
	{
		ProcessorQueue<Dummy> processor;

		called = false;
		processor.process([&](auto handlerContext)
		{
			// simulating some action
			std::chrono::milliseconds wait{10};
			std::this_thread::sleep_for(wait);

			// by this moment destructor has been called already
			handlerContext.getProcessor()->process([&](auto handlerContext)
			{
				called = true;
			});
		});
		EXPECT_FALSE(called);
	}
	EXPECT_TRUE(called);
}


TEST(ProcessorQueue, Flush1) {
	std::atomic<bool>     wasCalled;
	ProcessorQueue<Dummy> processor;

	wasCalled = false;
	processor.process([&](auto)
	{
		// simulating some action
		std::chrono::milliseconds wait{10};
		std::this_thread::sleep_for(wait);

		// setting called atomic flag
		wasCalled = true;
	});
	EXPECT_TRUE(wasCalled == false);
	processor.flush();
	EXPECT_TRUE(wasCalled == true);
}


TEST(ProcessorQueue, Process1) {
	std::atomic<bool>     wasCalled;
	ProcessorQueue<Dummy> processor;

	wasCalled = false;
	auto asyncCall = processor.process([&](auto)
	{
		// simulating some action
		std::chrono::milliseconds wait{10};
		std::this_thread::sleep_for(wait);

		// setting called atomic flag
		wasCalled = true;
	});
	EXPECT_TRUE(wasCalled == false);
	asyncCall.wait();
	EXPECT_TRUE(wasCalled == true);
}


TEST(ProcessorQueue, Process2) {
	std::atomic<int>      val;
	ProcessorQueue<Dummy> processor;

	val = 123;
	auto syncCall = processor.process([&](auto) -> int
	{
		// simulating some action
		std::chrono::milliseconds wait{10};
		std::this_thread::sleep_for(wait);

		return val;
	});
	EXPECT_EQ(val, syncCall.get());
}
