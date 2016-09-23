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


TEST(ProcessorAsio, Getters1)
{
	{
		conwrap::ProcessorAsio<Dummy> processor;

		EXPECT_TRUE(processor.getResource() != nullptr);
		EXPECT_EQ(&processor, processor.getResource()->processorPtr);
	}

	{
		auto dummyPtr    = std::make_unique<Dummy>();
		auto dummyRawPtr = dummyPtr.get();
		conwrap::ProcessorAsio<Dummy> processor(std::move(dummyPtr));

		EXPECT_EQ(dummyRawPtr, processor.getResource());
		EXPECT_EQ(&processor, processor.getResource()->processorPtr);
	}
}


TEST(ProcessorAsio, Destructor1) {
	std::atomic<bool> called;

	// processor's scope used to activate destructor before exiting test case
	{
		conwrap::ProcessorAsio<Dummy> processor;

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


TEST(ProcessorAsio, Destructor2)
{
	std::atomic<bool> called;

	// processor's scope used to activate destructor before exiting test case
	{
		conwrap::ProcessorAsio<Dummy> processor;

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


TEST(ProcessorAsio, Flush1)
{
	std::atomic<bool>             wasCalled;
	conwrap::ProcessorAsio<Dummy> processor;

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


TEST(ProcessorAsio, Flush2)
{
	std::thread::id id1;
	std::thread::id id2;
	{
		conwrap::ProcessorAsio<Dummy> processor;

		processor.process([&](auto)
		{
			id1 = std::this_thread::get_id();
		});
		processor.flush();
		processor.process([&](auto)
		{
			id2 = std::this_thread::get_id();
		});
		processor.flush();
		EXPECT_EQ(id1, id2);
	}
}


TEST(ProcessorAsio, Process1)
{
	std::atomic<bool>             wasCalled;
	conwrap::ProcessorAsio<Dummy> processor;

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


TEST(ProcessorAsio, Process2)
{
	std::atomic<int>              val;
	conwrap::ProcessorAsio<Dummy> processor;

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


TEST(ProcessorAsio, wrapHandler1)
{
	std::atomic<bool>             wasCalled;
	conwrap::ProcessorAsio<Dummy> processor;

	auto dispatcher = processor.getDispatcher();
	auto handler    = processor.wrapHandler([&]
	{
		// simulating some action
		std::chrono::milliseconds wait{10};
		std::this_thread::sleep_for(wait);

		// setting called atomic flag
		wasCalled = true;
	});

	wasCalled = false;
	dispatcher->post(handler);

	EXPECT_TRUE(wasCalled == false);
	processor.flush();
	EXPECT_TRUE(wasCalled == true);
}
