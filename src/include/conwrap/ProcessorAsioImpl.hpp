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

#include <asio.hpp>
#include <conwrap/HandlerContext.hpp>
#include <conwrap/HandlerWrapper.hpp>
#include <conwrap/Processor.hpp>
#include <conwrap/ProcessorQueue.hpp>
#include <conwrap/Task.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>


namespace conwrap
{
	// forward declaration
	template <typename ResourceType>
	class ProcessorAsioProxy;

	namespace internal
	{
		template <typename ResourceType>
		class ProcessorAsioImpl : public Processor<ResourceType, Task>
		{
			public:
				ProcessorAsioImpl(std::unique_ptr<ResourceType> r)
				: processorQueue(std::move(r)) {}

				virtual ~ProcessorAsioImpl() {}

				virtual HandlerContext<ResourceType> createContext() override
				{
					return HandlerContext<ResourceType> (getResource(), processorProxyPtr);
				}

				virtual void flush() override
				{
					// restarting dispatcher which will make sure all handlers are processed
					restartDispatcher();

					// making sure the main loop started after the restart
					this->process([] {}).wait();
				}

				asio::io_service* getDispatcher()
				{
					return &dispatcher;
				}

				virtual ResourceType* getResource() override
				{
					return processorQueue.getResource();
				}

				virtual void post(HandlerWrapper handlerWrapper) override
				{
					dispatcher.post(handlerWrapper);
				}

				void setProcessorProxy(ProcessorAsioProxy<ResourceType>* p)
				{
					processorProxyPtr = p;
				}

				void start()
				{
					{
						std::lock_guard<std::mutex> guard(lock);

						if (!thread.joinable())
						{
							// running dispacther on its dedicated thread
							thread = std::thread([&]
							{
								for (finished = false; !finished;)
								{
									// creating work object to make sure dispatcher performs until work object is deleted
									workPtr = std::make_unique<asio::io_service::work>(dispatcher);

									// the main processing loop
									while (processPending() > 0)
									{
										// queue must be flushed because it may generate new handlers
										processorQueue.flush();
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
							// deleting work object and setting finsihed flag
							post(wrapHandler([&]
							{
								workPtr.reset();
								finished = true;
							}));

							// waiting for the thread to finish
							thread.join();
						}
					}
				}

				void restartDispatcher()
				{
					{
						std::lock_guard<std::mutex> guard(lock);

						if (thread.joinable())
						{
							// reseting work object will make dispatcher exit as soon as there is no handler to process
							post(this->wrapHandler([&]
							{
								workPtr.reset();
							}));

						}
					}
				}

				virtual HandlerWrapper wrapHandler(std::function<void()> handler) override
				{
					return wrapHandler(handler, false);
				}

				virtual HandlerWrapper wrapHandler(std::function<void()> handler, bool proxy) override
				{
					return HandlerWrapper([=]
					{
						processorQueue.post(processorQueue.wrapHandler([=]
						{
							handler();
						}, proxy));
					}, proxy);
				}

			protected:
				auto processPending()
				{
					dispatcher.reset();
					return dispatcher.run();
				}

			private:
				ProcessorQueue<ResourceType>            processorQueue;
				ProcessorAsioProxy<ResourceType>*       processorProxyPtr;
				std::unique_ptr<asio::io_service::work> workPtr;
				asio::io_service                        dispatcher;
				std::thread                             thread;
				std::mutex                              lock;
				bool                                    finished;
		};
	}
}
