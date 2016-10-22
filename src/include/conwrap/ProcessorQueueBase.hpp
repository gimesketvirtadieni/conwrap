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
#include <conwrap/HandlerContext.hpp>
#include <conwrap/HandlerWrapper.hpp>
#include <conwrap/Processor.hpp>


namespace conwrap
{
	// forward declaration
	template <typename ResourceType>
	class ProcessorQueue;

	namespace internal
	{
		template <typename ResourceType>
		class ProcessorQueueBase : public ProcessorBase<ResourceType>
		{
			public:
				template <typename... Args>
				ProcessorQueueBase(std::unique_ptr<ResourceType> r)
				: resourcePtr(std::move(r))
				, epoch(1) {}

				virtual ~ProcessorQueueBase() {}

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

				virtual HandlerContext<ResourceType> createHandlerContext() override
				{
					return HandlerContext<ResourceType> (getResource(), processorPtr);
				}

				inline auto getEpoch()
				{
					return epoch;
				}

				virtual ResourceType* getResource() override
				{
					return resourcePtr.get();
				}

				virtual void post(HandlerWrapper handlerWrapper) override
				{
					queue.push(handlerWrapper);
				}

				void setProcessor(ProcessorQueue<ResourceType>* p)
				{
					processorPtr = p;

					// TODO: implemet compile-time reflection to make this invocation optional
					resourcePtr->setProcessor(processorPtr);
				}

				void start()
				{
					{
						std::lock_guard<std::mutex> guard(lock);

						if (!thread.joinable())
						{
							// it is safe to set finished flag here because worker thread is not created and start method is protected by a lock
							finished = false;
							thread   = std::thread([&]
							{
								for(; !(queue.empty() && finished);)
								{
									// waiting for a handler
									queue.wait();

									// executing handler
									if (auto handlerWrapperPtr = queue.get())
									{
										// TODO: work in progress
										// if this handler was submitted via non-proxy interface
										if (!handlerWrapperPtr->getProxy())
										{
											handlerWrapperPtr->setEpoch(epoch);

											// increasing epoch counter if this handler was submitted via non-proxy interface
											// TODO: MAX case must be handled
											epoch++;
										}

										// executing handler
										(*handlerWrapperPtr)();

										// setting epoch value for each newly created handler
										// TODO: this is not thread-safe!!!
										for (auto& h : queue)
										{
											if (h.getProxy() && !h.getEpoch())
											{
												h.setEpoch(handlerWrapperPtr->getEpoch());
											}
										}
									}

									// removing executed item
									queue.remove();
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
								// no need to protect with a lock because this assigment will be done on consumer's thread
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
					return HandlerWrapper(handler, proxy);
				}

			private:
				std::unique_ptr<ResourceType>   resourcePtr;
				ProcessorQueue<ResourceType>*   processorPtr;
				ConcurrentQueue<HandlerWrapper> queue;
				std::thread                     thread;
				std::mutex                      lock;
				bool                            finished;
				unsigned long long              epoch;
		};
	}
}
