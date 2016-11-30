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
#include <conwrap/HandlerWrapper.hpp>
#include <conwrap/Processor.hpp>
#include <conwrap/ProcessorQueue.hpp>
#include <conwrap/Provider.hpp>
#include <conwrap/TaskResult.hpp>
#include <conwrap/TaskResultProxy.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>


namespace conwrap
{
	// forward declaration
	template <typename ResourceType>
	class ProcessorAsio;
	template <typename ResourceType>
	class ProcessorAsioProxy;

	namespace internal
	{
		template <typename ResourceType>
		class ProcessorAsioImpl : public Processor<ResourceType>
		{
			// friend declaration
			template <typename> friend class conwrap::ProcessorAsio;
			template <typename> friend class conwrap::ProcessorAsioProxy;

			public:
				ProcessorAsioImpl(std::unique_ptr<ResourceType> r)
				: processorQueue(std::move(r)) {}

				virtual ~ProcessorAsioImpl() {}

				virtual void flush() override
				{
					// restarting dispatcher which will make sure all handlers are processed
					{
						std::lock_guard<std::mutex> guard(threadLock);

						if (thread.joinable())
						{
							// reseting work object will make dispatcher exit as soon as there is no handler to process
							this->process([&]
							{
								workPtr.reset();
							}).wait();

							// waiting for the main loop to restart
							{
								std::unique_lock<std::mutex> lock(workLock);
								conditionVariable.wait(lock, [&]
								{
									return (workPtr != nullptr);
								});
							}


						}
					}
				}

				inline asio::io_service* getDispatcher()
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

				virtual HandlerWrapper wrapHandler(std::function<void()> handler) override
				{
					return std::move(wrapHandler(std::move(handler), false));
				}

			protected:
				virtual Provider<ResourceType, TaskResult>* getProvider() override
				{
					return providerPtr.get();
				}

				inline Provider<ResourceType, TaskResultProxy>* getProviderProxy()
				{
					return providerProxyPtr.get();
				}

				auto processPending()
				{
					dispatcher.reset();
					return dispatcher.run();
				}

				inline void setProvider(Provider<ResourceType, TaskResult> t)
				{
					providerPtr = std::make_unique<Provider<ResourceType, TaskResult>>(t);
				}

				inline void setProviderProxy(Provider<ResourceType, TaskResultProxy> t)
				{
					providerProxyPtr = std::make_unique<Provider<ResourceType, TaskResultProxy>>(t);
				}

				void start()
				{
					{
						std::lock_guard<std::mutex> guard(threadLock);

						if (!thread.joinable())
						{
							// running dispacther on its dedicated thread
							thread = std::thread([&]
							{
								for (finished = false; !finished;)
								{
									// creating work object to make sure dispatcher performs until work object is deleted
									{
										std::lock_guard<std::mutex> lock(workLock);
										workPtr = std::make_unique<asio::io_service::work>(dispatcher);
									}
									conditionVariable.notify_all();

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
						std::lock_guard<std::mutex> guard(threadLock);

						if (thread.joinable())
						{
							// deleting work object and setting finsihed flag
							this->process([&]
							{
								finished = true;
								workPtr.reset();
							});

							// waiting for the thread to finish
							thread.join();
						}
					}
				}

				virtual HandlerWrapper wrapHandler(std::function<void()> handler, bool proxy) override
				{
					return std::move(HandlerWrapper([=]
					{
						processorQueue.post(std::move(processorQueue.wrapHandler([=]
						{
							handler();
						}, proxy)));
					}, proxy, 0));
				}

			private:
				std::unique_ptr<Provider<ResourceType, TaskResult>>      providerPtr;
				std::unique_ptr<Provider<ResourceType, TaskResultProxy>> providerProxyPtr;
				ProcessorQueue<ResourceType>                             processorQueue;
				asio::io_service                                         dispatcher;
				std::unique_ptr<asio::io_service::work>                  workPtr;
				std::mutex                                               workLock;
				std::condition_variable                                  conditionVariable;
				std::thread                                              thread;
				std::mutex                                               threadLock;
				bool                                                     finished;
		};
	}
}
