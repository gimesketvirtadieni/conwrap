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

#include <conwrap/HandlerContext.hpp>
#include <conwrap/ProcessorAsio.hpp>
#include <conwrap/ProcessorQueue.hpp>
#include <functional>
#include <iostream>


struct Dummy
{
	Dummy(conwrap::Processor<Dummy>*) {}
};


int main(int argc, char *argv[])
{
	{
		conwrap::ProcessorQueue<Dummy> processorQueue;
		processorQueue.process([](auto)
		{
			std::cout << "Hello from queue handler\n\r";
		});
		processorQueue.flush();
	}
	std::cout << "Back to main\n\r";

	{
		conwrap::ProcessorAsio<Dummy> processorAsio;
		processorAsio.process([](auto)
		{
			std::cout << "Hello from asio handler1\n\r";
		});
		processorAsio.flush();

		processorAsio.getDispatcher()->post(processorAsio.wrapHandler([] {
			std::cout << "Hello from asio handler2\n\r";
		}));
	}
	std::cout << "Back to main\n\r";

	return 0;
}
