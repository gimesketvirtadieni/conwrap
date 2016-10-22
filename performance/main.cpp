#include <iostream>
#include <sstream>
#include <fstream>
#include <ctime>
#include <cstdio>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <cassert>

#include <conwrap/Processor.hpp>
#include <conwrap/ProcessorQueue.hpp>
#include <conwrap/ProcessorAsio.hpp>


struct Dummy {
	Dummy() {}

	virtual ~Dummy() {}

	void setProcessor(conwrap::Processor<Dummy>* p)
	{
		processorPtr = p;
	}

	conwrap::Processor<Dummy>* processorPtr;
};


void generate(conwrap::Processor<Dummy>* processorPtr)
{
	for (int i = 0; i < 10000; i++)
	{
		processorPtr->process([] {});
	}
}


int main(int argc, char** argv) {
	auto processorQueuePtr = std::make_unique<conwrap::ProcessorQueue<Dummy>>();
	auto processorAsioPtr  = std::make_unique<conwrap::ProcessorAsio<Dummy>>();

	auto start_time = std::chrono::high_resolution_clock::now();
	generate(processorQueuePtr.get());
	processorQueuePtr->flush();
	auto end_time = std::chrono::high_resolution_clock::now();
	uint64_t duration = std::chrono::duration_cast<std::chrono::duration<uint64_t,std::ratio<1, 1000000>>>(end_time - start_time).count();
	std::cout << "processorQueue duration=" << duration << std::endl;

	start_time = std::chrono::high_resolution_clock::now();
	generate(processorAsioPtr.get());
	processorAsioPtr->flush();
	end_time = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::duration<uint64_t,std::ratio<1, 1000000>>>(end_time - start_time).count();
	std::cout << "processorAsio duration=" << duration << std::endl;

	return 0;
}
