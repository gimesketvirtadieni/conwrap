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
#include "HandlerWrapper.hpp"
#include "Processor.hpp"


template <typename ResourceType>
class ProcessorQueueImpl : public Processor<ResourceType>
{
	public:
		template <typename... Args>
		ProcessorQueueImpl(std::unique_ptr<ResourceType> r) : resourcePtr(std::move(r)) {}

		virtual ~ProcessorQueueImpl() {}

		virtual ResourceType* getResource() override
		{
			return resourcePtr.get();
		}

		virtual void flush() override
		{
			// waiting for the queue to become empty
			// TODO: this should be re-designed to use proper flush semantic
			while(!queue.empty())
			{
				this->process([](auto) {}).wait();
			}
		}

		virtual void post(std::function<void()> handler) override
		{
			queue.push(wrapHandler(handler));
		}

		void start()
		{
			{
				std::lock_guard<std::recursive_mutex> guard(lock);

				if (!thread.joinable())
				{
					thread = std::thread([&]
					{
						for(finished = false; !(queue.empty() && finished);)
						{
							HandlerWrapper handlerWrapper;

							// waiting for a handler
							queue.pop(handlerWrapper);

							// executing handler
							handlerWrapper();
						}
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
					this->process([&](auto)
					{
						// no need to protect with a lock because this assigment will be done on consumer's thread
						finished = true;
					});

					// waiting until consumer finsihes all pending handlers
					thread.join();
				}
			}
		}

		virtual HandlerWrapper wrapHandler(std::function<void()> handler) override
		{
			return HandlerWrapper(handler);
		}

	private:
		std::unique_ptr<ResourceType>   resourcePtr;
		ConcurrentQueue<HandlerWrapper> queue;
		std::thread                     thread;
		std::recursive_mutex            lock;
		bool                            finished;
};
