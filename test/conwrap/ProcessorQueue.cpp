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
		conwrap::ProcessorQueue<Dummy> processor;

		EXPECT_TRUE(processor.getResource() != nullptr);
		EXPECT_EQ(&processor, processor.getResource()->processorPtr);
	}

	{
		auto dummyPtr    = std::make_unique<Dummy>();
		auto dummyRawPtr = dummyPtr.get();
		conwrap::ProcessorQueue<Dummy> processor(std::move(dummyPtr));

		EXPECT_EQ(dummyRawPtr, processor.getResource());
		EXPECT_EQ(&processor, processor.getResource()->processorPtr);
	}
}


TEST(ProcessorQueue, Destructor1)
{
	std::atomic<bool> called;

	// processor's scope used to activate destructor before exiting test case
	{
		conwrap::ProcessorQueue<Dummy> processor;

		called = false;
		processor.process([&]
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


TEST(ProcessorQueue, Destructor2)
{
	std::atomic<bool> called;

	// processor's scope used to activate destructor before exiting test case
	{
		conwrap::ProcessorQueue<Dummy> processor;

		called = false;
		processor.process([&](auto context)
		{
			// simulating some action
			std::chrono::milliseconds wait{10};
			std::this_thread::sleep_for(wait);

			// by this moment destructor has been called already
			context.getProcessor()->process([&]
			{
				called = true;
			});
		});
		EXPECT_FALSE(called);
	}
	EXPECT_TRUE(called);
}


TEST(ProcessorQueue, Flush1)
{
	std::atomic<bool>              wasCalled;
	conwrap::ProcessorQueue<Dummy> processor;

	wasCalled = false;
	processor.process([&]
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


TEST(ProcessorQueue, Flush2)
{
	std::thread::id id1;
	std::thread::id id2;
	{
		conwrap::ProcessorQueue<Dummy> processor;

		processor.process([&]
		{
			id1 = std::this_thread::get_id();
		});
		processor.flush();
		processor.process([&]
		{
			id2 = std::this_thread::get_id();
		});
		processor.flush();
		EXPECT_EQ(id1, id2);
	}
}


TEST(ProcessorQueue, Flush3)
{
	conwrap::ProcessorQueue<Dummy> processor;

	processor.process([&](auto context)
	{
		// simulating some action
		std::chrono::milliseconds wait{5};
		std::this_thread::sleep_for(wait);

		context.getProcessor()->process([&]
		{
			// simulating some action
			std::chrono::milliseconds wait{10};
			std::this_thread::sleep_for(wait);
		});
	});

	auto asyncCall = std::async(
	    std::launch::async,
	    [&]
	    {
			// this delay will make sure that following handler is submitted AFTER the flush call
			std::chrono::milliseconds wait{10};
			std::this_thread::sleep_for(wait);

			processor.process([&]
			{
				// simulating some action
				std::chrono::milliseconds wait{10};
				std::this_thread::sleep_for(wait);
			});
	    }
	);
	processor.flush();
	asyncCall.wait();
}


TEST(ProcessorQueue, Process1)
{
	std::atomic<bool>              wasCalled;
	conwrap::ProcessorQueue<Dummy> processor;

	wasCalled = false;
	auto asyncCall = processor.process([&]
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


TEST(ProcessorQueue, Process2)
{
	std::atomic<int>               val;
	conwrap::ProcessorQueue<Dummy> processor;

	val = 123;
	auto syncCall = processor.process([&]() -> int
	{
		// simulating some action
		std::chrono::milliseconds wait{10};
		std::this_thread::sleep_for(wait);

		return val;
	});
	EXPECT_EQ(val, syncCall.get());
}


TEST(ProcessorQueue, Process3)
{
	std::atomic<void*>             ptr;
	conwrap::ProcessorQueue<Dummy> processor;

	processor.process([&](auto context)
	{
		ptr = context.getProcessor();
	}).wait();
	EXPECT_NE(&processor, ptr);
}
