#include <algorithm>
#include <cassert>
#include <chrono>
#include <conwrap/ConcurrentQueue.hpp>
#include <conwrap/HandlerWrapper.hpp>
#include <conwrap/Processor.hpp>
#include <conwrap/ProcessorQueue.hpp>
#include <conwrap/ProcessorAsio.hpp>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <vector>


struct Dummy {
	Dummy() {}

	virtual ~Dummy() {}

	void setProcessor(conwrap::Processor<Dummy>* p)
	{
		processorPtr = p;
	}

	conwrap::Processor<Dummy>* processorPtr;
};


void generate_baseline(conwrap::ConcurrentQueue<conwrap::HandlerWrapper>* queuePtr)
{
	for (int i = 0; i < 1000000; i++)
	{
		auto promisePtr = std::make_shared<std::promise<void>>();
		auto fun        = [] {};

		queuePtr->push(conwrap::HandlerWrapper([=]
		{
			fun();
			promisePtr->set_value();
		}, false, 0));

		// even though this shared future is not used, it is left here keep logic alike to processor
		std::shared_future<void>(promisePtr->get_future());
	}
}


void generate(conwrap::Processor<Dummy>* processorPtr)
{
	for (int i = 0; i < 1000000; i++)
	{
		processorPtr->process([] {});
	}
}


int main(int argc, char** argv) {
	auto queuePtr          = std::make_unique<conwrap::ConcurrentQueue<conwrap::HandlerWrapper>>();
	auto processorQueuePtr = std::make_unique<conwrap::ProcessorQueue<Dummy>>();
	auto processorAsioPtr  = std::make_unique<conwrap::ProcessorAsio<Dummy>>();

	std::thread thread([&]
	{
		// executing handlers
		for (int i = 0; i < 1000000; i++)
		{
			auto handler = queuePtr->front();
			handler();
			queuePtr->pop();
		}
	});
	auto start_time = std::chrono::high_resolution_clock::now();
	generate_baseline(queuePtr.get());
	thread.join();
	auto end_time = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::duration<uint64_t, std::milli>>(end_time - start_time);
	std::cout << "concurrentQueue duration=" << duration.count() << " millisec.\n\r";

	start_time = std::chrono::high_resolution_clock::now();
	generate(processorQueuePtr.get());
	processorQueuePtr->flush();
	end_time = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::duration<uint64_t, std::milli>>(end_time - start_time);
	std::cout << "processorQueue duration=" << duration.count() << " millisec.\n\r";

	start_time = std::chrono::high_resolution_clock::now();
	generate(processorAsioPtr.get());
	processorAsioPtr->flush();
	end_time = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::duration<uint64_t, std::milli>>(end_time - start_time);
	std::cout << "processorAsio duration=" << duration.count() << " millisec.\n\r";

	return 0;
}
