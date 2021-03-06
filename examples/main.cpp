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

#include <asio.hpp>
#include <chrono>
#include <conwrap/ProcessorAsio.hpp>
#include <conwrap/ProcessorAsioProxy.hpp>
#include <conwrap/ProcessorQueue.hpp>
#include <conwrap/ProcessorQueueProxy.hpp>
#include <conwrap/ProcessorProxy.hpp>
#include <functional>
#include <iostream>


class Server
{
	public:
		Server(short p)
		: processorProxyPtr(nullptr)
		, port(p) {}

		void close()
		{
			// closing acceptor
			acceptorPtr->close();

			// closing socket
			if (socketPtr->is_open())
			{
				try {
					socketPtr->shutdown(asio::socket_base::shutdown_both);
				} catch (...) {}
				try {
					socketPtr->close();
				} catch (...) {}
			}
		}

		void ping()
		{
			if (socketPtr->is_open())
			{
				socketPtr->send(asio::buffer("ping\n\r"));
				std::cout << "'ping' message was sent to client\n\r";
			}
		}

		void setProcessorProxy(conwrap::ProcessorAsioProxy<Server>* p)
		{
			processorProxyPtr = p;

			// creating an acceptor
			acceptorPtr = std::make_unique<asio::ip::tcp::acceptor>(
				*processorProxyPtr->getDispatcher(),
				asio::ip::tcp::endpoint(
					asio::ip::tcp::v4(),
					port
				)
			);

			// creating a socket
			socketPtr = std::make_unique<asio::ip::tcp::socket>(
				*processorProxyPtr->getDispatcher()
			);

			acceptorPtr->async_accept(
				*socketPtr,
				[=](const std::error_code error)
				{
					// wrapping is needed to pass handler from asio's thread to the processor's thread
					processorProxyPtr->wrap([=]
					{
						// start receiving data
						onData(error, 0);
					})();
				}
			);
		}

	protected:
		void onData(const std::error_code error, const std::size_t receivedSize)
		{
			if (!error)
			{
				// if there is data to send back to the client
				if (receivedSize > 0)
				{
					socketPtr->send(asio::buffer(buffer, receivedSize));
				}

				// keep receiving
				socketPtr->async_read_some(
					asio::buffer(buffer, BUFFER_SIZE),
					[=](const std::error_code error, const std::size_t receivedSize)
					{
						// wrapping is needed to pass handler from asio's thread to the processor's thread
						processorProxyPtr->wrap([=]
						{
							onData(error, receivedSize);
						})();
					}
				);
			}
		}

	private:
		conwrap::ProcessorAsioProxy<Server>*     processorProxyPtr;
		short                                    port;
		std::unique_ptr<asio::ip::tcp::acceptor> acceptorPtr;
		std::unique_ptr<asio::ip::tcp::socket>   socketPtr;
		enum {
			BUFFER_SIZE = 1024
		};
		char                                     buffer[BUFFER_SIZE];
};


int main(int argc, char *argv[])
{
	{
		conwrap::ProcessorQueue<bool> processorQueue;

		processorQueue.process([]
		{
			std::cout << "Hello from queue task\n\r";
		});
	}
	std::cout << "Back to main\n\r";

	// example demonstrating that Asio handlers are flushable now
	{
		conwrap::ProcessorAsio<bool> processorAsio;

		processorAsio.process([]
		{
			std::cout << "Hello from asio task\n\r";
		});
		processorAsio.flush();
		std::cout << "Back to main\n\r";
	}

	// example demonstrating composition of tasks
	{
		conwrap::ProcessorQueue<bool> processorQueue;

		processorQueue.process([]() -> int
		{
			std::cout << "Hello from the 1'st task\n\r";
			return 123;
		}).then([](auto context) -> bool
		{
			std::cout << "Hello from the 2'nd task (result from previous task is " << context.getResult() << ")\n\r";

			context.getProcessorProxy()->process([]() -> int
			{
				std::cout << "Hello from the 2.1'st task\n\r";
				return 1234;
			}).then([](auto context) -> bool
			{
				std::cout << "Hello from the 2.2'nd task (result from previous task is " << context.getResult() << ")\n\r";
				return true;
			}).then([](auto context)
			{
				std::cout << "Hello from the 2.3'rd task (result from previous task is " << context.getResult() << ")\n\r";
			});

			return false;
		}).then([](auto context)
		{
			std::cout << "Hello from the 3'rd task (result from previous task is " << context.getResult() << ")\n\r";
		});
		processorQueue.flush();
		std::cout << "Back to main\n\r";
	}

	// example demonstrating combining conwrap task with asio handlers
	{
		conwrap::ProcessorAsio<Server> processorAsio(1234);
		std::cout << "Echo server is listening on 1234 port (press ^C to terminate)\n\r";

		// now server runs on a separate thread; any other logic can be done here
		// submitting a task in a regular way
		processorAsio.process([]() -> int
		{
			std::cout << "Hello from asio task\n\r";

			// this is just to demonstrate that now asio can be used with tasks that return value
			return 1234;
		}).get();

		for (int i = 0; i < 5; i++)
		{
			// submitting a task directly via asio io_service
			processorAsio.getDispatcher()->post(

				// a task submitted directly via asio io_service must be wrapped as following
				processorAsio.wrap([&]
				{
					processorAsio.getResource()->ping();
				})
			);

			// sleeping
			std::this_thread::sleep_for(std::chrono::seconds{5});
		}

		// without this close invocation, flush would wait forever for any handlers
		// sync socket operations are thread safe since asio 1.4.0 so posting to asio thread is optional
		processorAsio.getResource()->close();

		// after this flush all tasks including asio handlers are executed
		processorAsio.flush();
		std::cout << "All tasks were executed\n\r";
	}

	return 0;
}
