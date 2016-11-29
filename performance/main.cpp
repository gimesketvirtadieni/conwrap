#include <chrono>
#include <conwrap/ConcurrentQueue.hpp>
#include <conwrap/Handler.hpp>
#include <conwrap/HandlerWrapper.hpp>
#include <conwrap/MoveOnCopy.hpp>
#include <conwrap/Processor.hpp>
#include <conwrap/ProcessorProxy.hpp>
#include <conwrap/ProcessorQueue.hpp>
#include <conwrap/ProcessorAsio.hpp>
#include <future>
#include <iostream>


// rewriting visibility of getFuture in conwrap::Handler
class HandlerDummy : public conwrap::Handler<std::function<void()>, void, bool>
{
	public:
		explicit HandlerDummy(std::function<void()> f)
		: Handler(std::move(f)) {}

		inline auto getFuture()
		{
			return conwrap::Handler<std::function<void()>, void, bool>::getFuture();
		}
};


void generate_baseline(conwrap::ConcurrentQueue<conwrap::HandlerWrapper>* queuePtr)
{
	for (int i = 0; i < 1000000; i++)
	{
		HandlerDummy handler(std::move([] {}));

		// even though this shared future is not used, it is left here keep logic alike to processor
		std::shared_future<void>(handler.getFuture());

		queuePtr->push(std::move(conwrap::HandlerWrapper(std::move(handler), false, 0)));
	}
}


void generate(conwrap::Processor<bool>* processorPtr)
{
	for (int i = 0; i < 1000000; i++)
	{
		processorPtr->process([] {});
	}
}


int main(int argc, char** argv) {
	auto queuePtr          = std::make_unique<conwrap::ConcurrentQueue<conwrap::HandlerWrapper>>();
	auto processorQueuePtr = std::make_unique<conwrap::ProcessorQueue<bool>>();
	auto processorAsioPtr  = std::make_unique<conwrap::ProcessorAsio<bool>>();

	std::chrono::duration<uint64_t, std::milli>         duration;
	decltype(std::chrono::high_resolution_clock::now()) start_time;
	decltype(std::chrono::high_resolution_clock::now()) end_time;

	std::thread thread([&]
	{
		// executing handlers
		for (int i = 0; i < 1000000; i++)
		{
			conwrap::HandlerWrapper& handler = queuePtr->front();
			handler();
			queuePtr->pop();
		}
	});
	start_time = std::chrono::high_resolution_clock::now();
	generate_baseline(queuePtr.get());
	thread.join();
	end_time = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::duration<uint64_t, std::milli>>(end_time - start_time);
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
