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

#include <memory>
#include <thread>
#include "ConcurrentQueue.hpp"
#include "HandlerContext.hpp"
#include "Processor.hpp"


template <typename ResourceType>
class ProcessorQueueImpl : public Processor<ResourceType>
{
	public:
		template <typename... Args>
		ProcessorQueueImpl(std::unique_ptr<ResourceType> r) : resourcePtr(std::move(r)) {}

		virtual ~ProcessorQueueImpl() {}

		virtual ResourceType* getResource()
		{
			return resourcePtr.get();
		}

		virtual void flush()
		{
			// waiting for the queue to become empty
			// TODO: this should be re-designed to use proper flush semantic
			while(!queue.empty())
			{
				this->process([](auto) {}).wait();
			}
		}

		virtual void post(std::function<void()> handler)
		{
			queue.push(handler);
		}

		void start()
		{
			if (!thread.joinable())
			{
				std::lock_guard<std::recursive_mutex> guard(lock);

				thread = std::thread([&]
				{
					for(finished = false; !(queue.empty() && finished);)
					{
						std::function<void()> handler;

						// waiting for a handler
						queue.pop(handler);

						// executing handler
						handler();
					}
				});
			}
		}

		void stop()
		{
			if (thread.joinable())
			{
				std::lock_guard<std::recursive_mutex> guard(lock);

				this->process([&](auto)
				{
					// no need to protect with a lock because this assigment will be done on consumer's thread
					finished = true;
				});

				// waiting until consumer finsihes all pending handlers
				thread.join();
			}
		}

	private:
		std::unique_ptr<ResourceType>          resourcePtr;
		ConcurrentQueue<std::function<void()>> queue;
		std::thread                            thread;
		std::recursive_mutex                   lock;
		bool                                   finished;
};
