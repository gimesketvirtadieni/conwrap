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
#include <conwrap/ProcessorQueue.hpp>
#include <functional>
#include <iostream>


struct Dummy
{
	// TODO: this is a temporary fix before reflection is implemented
	void setProcessor(conwrap::Processor<Dummy>*) {}
};


class Server
{
	public:
		Server(short p) : port(p) {}

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
			}
		}

		void setProcessor(conwrap::ProcessorAsio<Server>* p)
		{
			processorPtr = p;

			// creating an acceptor
			acceptorPtr = std::make_unique<asio::ip::tcp::acceptor>(
				*processorPtr->getDispatcher(),
				asio::ip::tcp::endpoint(
					asio::ip::tcp::v4(),
					port
				)
			);

			// creating a socket
			socketPtr = std::make_unique<asio::ip::tcp::socket>(
				*processorPtr->getDispatcher()
			);

			acceptorPtr->async_accept(
				*socketPtr,
				[=](const std::error_code error)
				{
					// wrapping is needed to pass handler from asio's thread to the processor's thread
					processorPtr->wrapHandler([=]
					{
						// start receiving data
						onData(error, 0);
					})();
				}
			);
		}

		// TODO: this is a temporary fix before reflection is implemented
		void setProcessor(conwrap::ProcessorQueue<Server>*) {}

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
						processorPtr->wrapHandler([=]
						{
							onData(error, receivedSize);
						})();
					}
				);
			}
		}

	private:
		conwrap::ProcessorAsio<Server>*          processorPtr;
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
		conwrap::ProcessorQueue<Dummy> processorQueue;

		processorQueue.process([]
		{
			std::cout << "Hello from queue task\n\r";
		});
	}
	std::cout << "Back to main\n\r";

	{
		conwrap::ProcessorAsio<Dummy> processorAsio;

		processorAsio.process([]
		{
			std::cout << "Hello from asio task\n\r";
		});
		processorAsio.flush();
		std::cout << "Back to main\n\r";
	}

	{
		conwrap::ProcessorAsio<Server> processorAsio(1234);
		std::cout << "Echo server is listening on 1234 port (press ^C to terminate)\n\r";

		// now server runs on a separate thread; any other logic can be done here
		for (int i = 0; i < 3; i++)
		{
			// submitting a task in a regular way
			processorAsio.process([]() -> int
			{
				std::cout << "Hello from asio task\n\r";

				// this is just to demonstrate that now asio can be used with tasks that return value
				return 1234;
			});

			for (int j = 0; j < 3; j++)
			{
				// submitting a task directly via asio io_service
				processorAsio.getDispatcher()->post(

					// a task submitted directly via asio io_service must be wrapped as following
					processorAsio.wrapHandler([&]
					{
						processorAsio.getResource()->ping();
					})
				);

				// sleeping
				std::this_thread::sleep_for(std::chrono::seconds{5});
			}

			// after this flush all tasks including asio handlers are executed
			processorAsio.flush();
			std::cout << "All tasks were flushed\n\r";
		}

		// without this close processor's destructor would wait forever for async receive handler
		// sync socket operations are thread safe since asio 1.4.0 so posting to asio thread is optional
		processorAsio.getDispatcher()->post([&]
		{
			 processorAsio.getResource()->close();
		});

		// this flush is required as handler posted to asio captured processorAsio reference
		processorAsio.flush();
	}

	return 0;
}
