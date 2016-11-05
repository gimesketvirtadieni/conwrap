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


TEST_P(ProcessorCommon, Getters1)
{
	auto processorPtr = GetParam();

	EXPECT_TRUE(processorPtr->getResource() != nullptr);
}


TEST_P(ProcessorCommon, Flush1)
{
	auto              processorPtr(GetParam());
	std::atomic<bool> wasCalled(false);
	void*             resourcePtr(nullptr);
	void*             processorProxyPtr(nullptr);

	processorPtr->process([&](auto context)
	{
		// simulating some action
		std::this_thread::sleep_for(std::chrono::milliseconds{10});

		wasCalled         = true;
		resourcePtr       = context.getResource();
		processorProxyPtr = context.getProcessorProxy();
	});
	processorPtr->flush();
	EXPECT_TRUE(wasCalled == true);
	EXPECT_EQ(processorPtr->getResource(), resourcePtr);
	// TODO: implement getProcessorProxy via fixture
	//EXPECT_EQ(processorPtr->getProcessorProxy(), processorProxyPtr);
}


TEST_P(ProcessorCommon, Flush2)
{
	auto              processorPtr(GetParam());
	std::atomic<bool> wasCalled(false);
	void*             resourcePtr(nullptr);
	void*             processorProxyPtr(nullptr);

	processorPtr->process([&](auto context)
	{
		// simulating some action
		std::this_thread::sleep_for(std::chrono::milliseconds{10});

		context.getProcessorProxy()->process([&](auto context)
		{
			// simulating some action
			std::this_thread::sleep_for(std::chrono::milliseconds{10});

			wasCalled         = true;
			resourcePtr       = context.getResource();
			processorProxyPtr = context.getProcessorProxy();
		});
	});
	processorPtr->flush();
	EXPECT_TRUE(wasCalled == true);
	EXPECT_EQ(processorPtr->getResource(), resourcePtr);
	// TODO: implement getProcessorProxy via fixture
	//EXPECT_EQ(processorPtr->getProcessorProxy(), processorProxyPtr);
}


TEST_P(ProcessorCommon, Flush3)
{
	auto            processorPtr(GetParam());
	std::thread::id id1;
	std::thread::id id2;

	processorPtr->process([&]
	{
		id1 = std::this_thread::get_id();
	});
	processorPtr->process([&]
	{
		id2 = std::this_thread::get_id();
	});
	processorPtr->flush();
	EXPECT_EQ(id1, id2);
}


TEST_P(ProcessorCommon, Process1)
{
	auto              processorPtr(GetParam());
	std::atomic<bool> wasCalled(false);

	processorPtr->process([&]
	{
		wasCalled = true;
	}).wait();
	EXPECT_EQ(wasCalled, true);
}


TEST_P(ProcessorCommon, Process2)
{
	auto             processorPtr(GetParam());
	std::atomic<int> val(123);

	auto result = processorPtr->process([&]() -> int
	{
		return val;
	}).getResult();
	EXPECT_EQ(val, result);
}


TEST_P(ProcessorCommon, Process3)
{
	auto               processorPtr(GetParam());
	std::atomic<void*> ptr(nullptr);

	processorPtr->process([&](auto context)
	{
		ptr = context.getProcessorProxy();
	}).wait();
	EXPECT_NE(processorPtr.get(), ptr);
}


TEST_P(ProcessorCommon, Process4)
{
	auto              processorPtr(GetParam());
	std::atomic<bool> wasCalled(false);

	processorPtr->getResource()->processorProxyPtr->process([&](auto context)
	{
		wasCalled = true;
	});
	processorPtr->flush();
	EXPECT_TRUE(wasCalled);
}


TEST_P(ProcessorCommon, Process5)
{
	auto  processorPtr(GetParam());
	Dummy dummy;
	{
		testing::InSequence dummySequence;

		EXPECT_CALL(dummy, method1())
			.Times(1);
		EXPECT_CALL(dummy, method2())
			.Times(1);
	}

	processorPtr->process([&]
	{
		dummy.method1();
	}).then([&]
	{
		dummy.method2();
	}).wait();
}


TEST_P(ProcessorCommon, Process6)
{
	auto  processorPtr(GetParam());
	Dummy dummy;
	{
		testing::InSequence dummySequence;

		EXPECT_CALL(dummy, method1())
			.Times(1);
		EXPECT_CALL(dummy, method2())
			.Times(1);
	}

	processorPtr->process([&](auto context)
	{
		// simulating some action
		std::this_thread::sleep_for(std::chrono::milliseconds{10});

		context.getProcessorProxy()->process([&]
		{
			// simulating some action
			std::this_thread::sleep_for(std::chrono::milliseconds{10});
		});
	}).then([&](auto context)
	{
		context.getProcessorProxy()->process([&]
		{
			// simulating some action
			std::this_thread::sleep_for(std::chrono::milliseconds{10});

			dummy.method1();
		}).then([&]
		{
			// simulating some action
			std::this_thread::sleep_for(std::chrono::milliseconds{10});

			dummy.method2();
		});
	});
	processorPtr->flush();
}


TEST_P(ProcessorCommon, Process7)
{
	auto             processorPtr(GetParam());
	std::atomic<int> result(0);

	processorPtr->process([&]() -> int
	{
		return 1234;
	}).then([&](auto context)
	{
		result = context.getResult();
	}).wait();
	EXPECT_EQ(result, 1234);
}


TEST_P(ProcessorCommon, Process8)
{
	auto             processorPtr(GetParam());
	std::atomic<int> result(0);

	processorPtr->process([&]
	{
		// simulating some action
		std::this_thread::sleep_for(std::chrono::milliseconds{10});
	}).then([&](auto context)
	{
		// simulating some action
		std::this_thread::sleep_for(std::chrono::milliseconds{10});

		context.getProcessorProxy()->process([&]() -> int
		{
			result = 1234;
			return 1234;
		}).then([&](auto context)
		{
			result = context.getResult();
		});
	});
	processorPtr->flush();
	EXPECT_EQ(result, 1234);
}


INSTANTIATE_TEST_CASE_P(ProcessorInstantiation, ProcessorCommon, ::testing::Values(
	std::make_shared<conwrap::ProcessorMock>(),
	std::make_shared<conwrap::ProcessorQueue<Dummy>>(),
	std::make_shared<conwrap::ProcessorAsio<Dummy>>()
));
