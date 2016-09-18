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

#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <mutex>
#include <thread>
#include "HandlerContext.hpp"
#include "HandlerWrapper.hpp"
#include "Processor.hpp"
#include "ProcessorQueue.hpp"


template <typename ResourceType>
class ProcessorAsioImpl : public Processor<ResourceType>
{
	public:
		template <typename... Args>
		ProcessorAsioImpl(std::unique_ptr<ResourceType> r)
		: processorQueue(std::move(r)) {}

		virtual ~ProcessorAsioImpl() {}

		boost::asio::io_service* getDispatcher()
		{
			return &dispatcher;
		}

		virtual ResourceType* getResource() override
		{
			return processorQueue.getResource();
		}

		virtual void flush() override
		{
			{
				std::lock_guard<std::recursive_mutex> guard(lock);

				if (thread.joinable())
				{
					// stopping dispatcher will make sure all handlers are in processor queue
					stop();

					// start processing as flush will not be affected by incomming handlers
					start();
				}
			}

			// flushing processor's queue
			processorQueue.flush();
		}

		virtual void post(std::function<void()> handler) override
		{
			dispatcher.post(wrapHandler(handler));
		}

		void start()
		{
			{
				std::lock_guard<std::recursive_mutex> guard(lock);

				if (!thread.joinable())
				{
					// creating work object to make sure dispatcher performs until work object is deleted
					workPtr = std::make_unique<boost::asio::io_service::work>(dispatcher);

					// running dispacther on its dedicated thread
					thread = std::thread([&]
					{
						dispatcher.run();
					});
				}
			}
		}

		void stop()
		{
			{
				std::lock_guard<std::recursive_mutex> guard(lock);

				if (thread.joinable())
				{
					// reseting work object will make dispatcher exit as soon as there is no handler to process
					workPtr.reset();

					// waiting for the thread to finish
					thread.join();

					// cleanning up
					dispatcher.reset();
				}
			};
		}

		virtual HandlerWrapper wrapHandler(std::function<void()> handler) override
		{
			return HandlerWrapper([=]
			{
				processorQueue.process([=](auto)
				{
					handler();
				});
			});
		}

	private:
		ProcessorQueue<ResourceType>                   processorQueue;
		std::unique_ptr<boost::asio::io_service::work> workPtr;
		boost::asio::io_service                        dispatcher;
		std::thread                                    thread;
		std::recursive_mutex                           lock;
};
