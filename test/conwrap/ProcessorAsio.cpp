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
#include <thread>
#include "Mocks.hpp"


TEST(ProcessorAsio, Constructor1)
{
	auto dummyPtr    = std::make_unique<Dummy>();
	auto dummyRawPtr = dummyPtr.get();
	conwrap::ProcessorAsio<Dummy> processor(std::move(dummyPtr));

	EXPECT_EQ(dummyRawPtr, processor.getResource());
	EXPECT_NE(nullptr, processor.getResource()->processorPtr);
}


TEST(ProcessorAsio, Destructor1) {
	std::atomic<bool> called(false);

	// processor's scope used to activate destructor before exiting test case
	{
		conwrap::ProcessorAsio<Dummy> processor;

		processor.process([&]
		{
			// simulating some action
			std::this_thread::sleep_for(std::chrono::milliseconds{10});

			// by this moment destructor has been called already
			called = true;
		});
	}
	EXPECT_TRUE(called);
}


TEST(ProcessorAsio, Destructor2)
{
	std::atomic<bool> called(false);

	// processor's scope used to activate destructor before exiting test case
	{
		conwrap::ProcessorAsio<Dummy> processor;

		processor.process([&](auto context)
		{
			// simulating some action
			std::this_thread::sleep_for(std::chrono::milliseconds{10});

			// by this moment destructor has been called already
			context.getProcessor()->process([&]
			{
				called = true;
			});
		});
	}
	EXPECT_TRUE(called);
}


TEST(ProcessorAsio, wrapHandler1)
{
	conwrap::ProcessorAsio<Dummy> processor;
	std::atomic<bool>             wasCalled(false);

	auto dispatcher = processor.getDispatcher();
	auto handler    = processor.wrapHandler([&]
	{
		// simulating some action
		std::this_thread::sleep_for(std::chrono::milliseconds{10});

		// by this moment flush has been called already
		wasCalled = true;
	});

	dispatcher->post(handler);
	processor.flush();
	EXPECT_TRUE(wasCalled == true);
}
