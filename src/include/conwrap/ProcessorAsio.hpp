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
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <conwrap/HandlerContext.hpp>
#include <conwrap/HandlerWrapper.hpp>
#include <conwrap/Processor.hpp>
#include <conwrap/ProcessorQueue.hpp>


namespace conwrap
{

	template <typename ResourceType>
	class ProcessorAsio : public Processor<ResourceType>
	{
		public:
			template <typename... Args>
			ProcessorAsio(Args... args)
			: processorImplPtr(std::make_shared<ProcessorAsioImpl<ResourceType>>(std::move(std::make_unique<ResourceType>(this, std::forward<Args>(args)...))))
			, processorProxyPtr(std::unique_ptr<ProcessorAsio<ResourceType>>(new ProcessorAsio<ResourceType>(processorImplPtr)))
			, proxy(false)
			{
				processorImplPtr->start();
			}

			template <typename... Args>
			ProcessorAsio(std::unique_ptr<ResourceType> resource)
			: processorImplPtr(std::make_shared<ProcessorAsioImpl<ResourceType>>(std::move(resource)))
			, processorProxyPtr(std::unique_ptr<ProcessorAsio<ResourceType>>(new ProcessorAsio<ResourceType>(processorImplPtr)))
			, proxy(false)
			{
				processorImplPtr->start();
			}

			virtual ~ProcessorAsio()
			{
				if (!proxy)
				{
					processorImplPtr->stop();
				}
			}

			boost::asio::io_service* getDispatcher()
			{
				return processorImplPtr->getDispatcher();
			}

			virtual ResourceType* getResource() override
			{
				return processorImplPtr->getResource();
			}

			virtual void flush() override
			{
				processorImplPtr->flush();
			}

			virtual HandlerWrapper wrapHandler(std::function<void()> handler) override
			{
				return processorImplPtr->wrapHandler(handler);
			}

		protected:
			template <typename ResourceType2>
			class ProcessorAsioImpl : public Processor<ResourceType2>
			{
				public:
					template <typename... Args>
					ProcessorAsioImpl(std::unique_ptr<ResourceType2> r)
					: processorQueue(std::move(r)) {}

					virtual ~ProcessorAsioImpl() {}

					boost::asio::io_service* getDispatcher()
					{
						return &dispatcher;
					}

					virtual ResourceType2* getResource() override
					{
						return processorQueue.getResource();
					}

					virtual void flush() override
					{
						{
							std::lock_guard<std::recursive_mutex> guard(lock);

							// TODO: redesign to avoid invoking stop-start as it changes running thread
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
									// TODO: rewrite...
									dispatcher.reset();
									auto processedCount = dispatcher.run();

									for (processedCount = 1 ;processedCount > 0;)
									{
										processorQueue.flush();
										dispatcher.reset();
										processedCount = dispatcher.run();
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
								// reseting work object will make dispatcher exit as soon as there is no handler to process
								workPtr.reset();

								// waiting for the thread to finish
								thread.join();
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
					ProcessorQueue<ResourceType2>                  processorQueue;
					std::unique_ptr<boost::asio::io_service::work> workPtr;
					boost::asio::io_service                        dispatcher;
					std::thread                                    thread;
					std::recursive_mutex                           lock;
			};

			ProcessorAsio(std::shared_ptr<ProcessorAsioImpl<ResourceType>> processorImplPtr)
			: processorImplPtr(processorImplPtr)
			, proxy(true) {}

			virtual void post(std::function<void()> handler) override
			{
				processorImplPtr->post(handler);
			}

		private:
			std::shared_ptr<ProcessorAsioImpl<ResourceType>> processorImplPtr;
			std::unique_ptr<ProcessorAsio<ResourceType>>     processorProxyPtr;
			bool                                             proxy;
	};

}
