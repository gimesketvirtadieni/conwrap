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


TEST(Processor, Constructor1)
{
	auto dummyPtr    = std::make_unique<Dummy>();
	auto dummyRawPtr = dummyPtr.get();
	conwrap::ProcessorMock processor(std::move(dummyPtr));

	EXPECT_EQ(dummyRawPtr, processor.getResource());
	EXPECT_NE(nullptr, processor.getResource()->processorPtr);
}
