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
#include <future>
#include "Mocks.hpp"


TEST(ProcessorQueue, Constructor1)
{
	auto dummyPtr    = std::make_unique<Dummy>();
	auto dummyRawPtr = dummyPtr.get();
	conwrap::ProcessorQueue<Dummy> processor(std::move(dummyPtr));

	EXPECT_EQ(dummyRawPtr, processor.getResource());
	EXPECT_NE(nullptr, processor.getResource()->processorPtr);
}


TEST(ProcessorQueue, Destructor1)
{
	std::atomic<bool> called(false);

	// processor's scope used to activate destructor before exiting test case
	{
		conwrap::ProcessorQueue<Dummy> processor;

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


TEST(ProcessorQueue, Destructor2)
{
	std::atomic<bool> called(false);

	// processor's scope used to activate destructor before exiting test case
	{
		conwrap::ProcessorQueue<Dummy> processor;

		processor.process([&](auto context)
		{
			// simulating some action
			std::this_thread::sleep_for(std::chrono::milliseconds{10});

			// by this moment destructor has been called already
			context.getProcessorProxy()->process([&]
			{
				called = true;
			});
		});
	}
	EXPECT_TRUE(called);
}


// TODO: refactor to avoid time-based syncing
/* work in progress
TEST(ProcessorQueue, Flush3)
{
	conwrap::ProcessorQueue<Dummy> processor;

	auto ready = std::promise<void>();

	processor.process([&](auto context)
	{
		context.getProcessorProxy()->process([&]
		{
			// simulating some action
			std::cout << "Second handler start\n\r";
			std::chrono::milliseconds wait{5};
			std::this_thread::sleep_for(wait);
			std::cout << "Second handler end\n\r";
		});

		// waiting for all tasks to be submitted
		ready.get_future().wait();

		// simulating some action
		std::cout << "First handler start\n\r";
		std::chrono::milliseconds wait{5};
		std::this_thread::sleep_for(wait);
		std::cout << "First handler end\n\r";
	});

	auto asyncCall = std::async(
	    std::launch::async,
	    [&]
	    {
			// this delay will make sure that following handler is submitted AFTER the flush call
			std::chrono::milliseconds wait{5};
			std::this_thread::sleep_for(wait);

			processor.process([&]
			{
				// simulating some action; it needs to be longer than #1 & #2 so flush has a chance to complete
				std::cout << "Third handler start\n\r";
				std::chrono::milliseconds wait{5};
				std::this_thread::sleep_for(wait);
				std::cout << "Third handler end\n\r";
			});

			// all tasks were submitted; start processing
			ready.set_value();
	    }
	);

	std::cout << "HELLO before flush\n\r";
	processor.flush();
	std::cout << "HELLO after flush\n\r";
	asyncCall.wait();
	std::cout << "HELLO done\n\r";
}
*/
