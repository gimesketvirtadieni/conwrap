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
		context.getProcessorProxy()->process([&](auto context)
		{
			wasCalled         = true;
			resourcePtr       = context.getResource();
			processorProxyPtr = context.getProcessorProxy();
		});
	}).wait();
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

	auto asyncCall = processorPtr->process([&]
	{
		wasCalled = true;
	});
	asyncCall.wait();
	EXPECT_TRUE(wasCalled == true);
}


TEST_P(ProcessorCommon, Process2)
{
	auto             processorPtr(GetParam());
	std::atomic<int> val(123);

	auto syncCall = processorPtr->process([&]() -> int
	{
		return val;
	});
	EXPECT_EQ(val, syncCall.get());
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

	processorPtr->getResource()->processorPtr->process([&](auto context)
	{
		wasCalled = true;
	}).wait();
	EXPECT_TRUE(wasCalled);
}


INSTANTIATE_TEST_CASE_P(ProcessorInstantiation, ProcessorCommon, ::testing::Values(
	std::make_shared<conwrap::ProcessorMock>(),
	std::make_shared<conwrap::ProcessorQueue<Dummy>>(),
	std::make_shared<conwrap::ProcessorAsio<Dummy>>()
));
