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

#include <conwrap/ConcurrentQueue.hpp>
#include <conwrap/HandlerContext.hpp>
#include <conwrap/HandlerWrapper.hpp>
#include <conwrap/Processor.hpp>
#include <conwrap/Task.hpp>
#include <conwrap/TaskProvider.hpp>
#include <conwrap/TaskProxy.hpp>
#include <memory>
#include <thread>


namespace conwrap
{
	// forward declaration
	template <typename ResourceType>
	class ProcessorQueue;
	template <typename ResourceType>
	class ProcessorQueueProxy;

	namespace internal
	{
		template <typename ResourceType>
		class ProcessorQueueImpl : public Processor<ResourceType>
		{
			// friend declaration
			template <typename> friend class conwrap::ProcessorQueue;
			template <typename> friend class conwrap::ProcessorQueueProxy;

			public:
				ProcessorQueueImpl(std::unique_ptr<ResourceType> r)
				: resourcePtr(std::move(r))
				, nextEpoch(1) {}

				virtual ~ProcessorQueueImpl() {}

				virtual void flush() override
				{
					// figuring out current epoch
					auto currentEpoch = this->process([&]() -> auto
					{
						return getNextEpoch();
					}).getResult();

					// waiting for all 'child' handlers to be processed
					while (childExists(currentEpoch))
					{
						// TODO: insert flush handler after the last child instead of adding at the end of the queue
						this->process([=] {}).wait();
					}
				}

				virtual ResourceType* getResource() override
				{
					return resourcePtr.get();
				}

			protected:
				bool childExists(unsigned long long currentEpoch)
				{
					auto found = false;

					for (auto& h : queue)
					{
						auto epoch = h.getEpoch();
						if (h.getProxy() && epoch && epoch < currentEpoch)
						{
							found = true;
							break;
						}
					}
					return found;
				}

				inline auto getNextEpoch()
				{
					return nextEpoch;
				}

				virtual TaskProvider<ResourceType, Task>* getTaskProvider() override
				{
					return taskProviderPtr.get();
				}

				inline TaskProvider<ResourceType, TaskProxy>* getTaskProxyProvider()
				{
					return taskProxyProviderPtr.get();
				}

				virtual void post(HandlerWrapper handlerWrapper) override
				{
					queue.push(handlerWrapper);
				}

				inline void setTaskProvider(TaskProvider<ResourceType, Task> t)
				{
					taskProviderPtr = std::make_unique<TaskProvider<ResourceType, Task>>(t);
				}

				inline void setTaskProxyProvider(TaskProvider<ResourceType, TaskProxy> t)
				{
					taskProxyProviderPtr = std::make_unique<TaskProvider<ResourceType, TaskProxy>>(t);
				}

				void start()
				{
					{
						std::lock_guard<std::mutex> guard(lock);

						if (!thread.joinable())
						{
							// it is safe to set finished flag on consumer's thread because worker's thread is not created and start method is protected by a lock
							finished = false;
							thread   = std::thread([&]
							{
								for(; !(queue.empty() && finished);)
								{
									// waiting for a handler
									if (auto handlerPtr = queue.get())
									{
										// setting current epoch to be used for submitted tasks via processor proxy
										currentEpoch = handlerPtr->getEpoch();

										// executing handler
										(*handlerPtr)();

										// removing executed item
										queue.remove();
									}
								}
							});
						}
					}
				}

				void stop()
				{
					{
						std::lock_guard<std::mutex> guard(lock);

						if (thread.joinable())
						{
							post(wrapHandler([&]
							{
								// no need to protect with a lock because this assigment will be done on worker's thread
								finished = true;
							}));

							// waiting until consumer finsihes all pending handlers
							thread.join();
						}
					}
				}

				virtual HandlerWrapper wrapHandler(std::function<void()> handler) override
				{
					return wrapHandler(handler, false);
				}

				virtual HandlerWrapper wrapHandler(std::function<void()> handler, bool proxy) override
				{
					// TODO: MAX case must be handled
					return HandlerWrapper(handler, proxy, (proxy ? currentEpoch : nextEpoch++));
				}

			private:
				std::unique_ptr<ResourceType>                          resourcePtr;
				std::unique_ptr<TaskProvider<ResourceType, Task>>      taskProviderPtr;
				std::unique_ptr<TaskProvider<ResourceType, TaskProxy>> taskProxyProviderPtr;
				ConcurrentQueue<HandlerWrapper>                        queue;
				std::thread                                            thread;
				std::mutex                                             lock;
				bool                                                   finished;
				unsigned long long                                     nextEpoch;
				unsigned long long                                     currentEpoch;
		};
	}
}
