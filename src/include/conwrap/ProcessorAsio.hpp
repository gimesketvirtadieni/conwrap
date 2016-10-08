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
			: processorImplPtr(std::make_shared<ProcessorAsioImpl<ResourceType>>(std::move(std::make_unique<ResourceType>(std::forward<Args>(args)...))))
			, processorProxyPtr(std::unique_ptr<ProcessorAsio<ResourceType>>(new ProcessorAsio<ResourceType>(processorImplPtr)))
			, proxy(false)
			{
				processorImplPtr->setProcessor(processorProxyPtr.get());
				processorImplPtr->start();
			}

			template <typename... Args>
			ProcessorAsio(std::unique_ptr<ResourceType> resource)
			: processorImplPtr(std::make_shared<ProcessorAsioImpl<ResourceType>>(std::move(resource)))
			, processorProxyPtr(std::unique_ptr<ProcessorAsio<ResourceType>>(new ProcessorAsio<ResourceType>(processorImplPtr)))
			, proxy(false)
			{
				processorImplPtr->setProcessor(processorProxyPtr.get());
				processorImplPtr->start();
			}

			virtual ~ProcessorAsio()
			{
				if (!proxy)
				{
					processorImplPtr->stop();
				}
			}

			virtual HandlerContext<ResourceType> createHandlerContext() override
			{
				return processorImplPtr->createHandlerContext();
			}

			asio::io_service* getDispatcher()
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

			virtual void post(HandlerWrapper handlerWrapper) override
			{
				processorImplPtr->post(handlerWrapper);
			}

			virtual HandlerWrapper wrapHandler(std::function<void()> handler)
			{
				return wrapHandler(handler, proxy);
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

					virtual HandlerContext<ResourceType> createHandlerContext() override
					{
						return HandlerContext<ResourceType> (getResource(), processorPtr);
					}

					asio::io_service* getDispatcher()
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
							std::lock_guard<std::mutex> guard(lock);

							if (thread.joinable())
							{
								// reseting work object will make dispatcher exit as soon as there is no handler to process
								post(this->wrapHandler([&]
								{
									workPtr.reset();
								}));

								// waiting for the main loop to restart
								this->process([] {}).wait();
							}
						}
					}

					virtual void post(HandlerWrapper handlerWrapper) override
					{
						dispatcher.post(handlerWrapper);
					}

					void setProcessor(ProcessorAsio<ResourceType2>* p)
					{
						processorPtr = p;

						// TODO: implemet compile-time reflection to make this invocation optional
						processorQueue.getResource()->setProcessor(processorPtr);
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
					ProcessorQueue<ResourceType2>           processorQueue;
					ProcessorAsio<ResourceType2>*           processorPtr;
					std::unique_ptr<asio::io_service::work> workPtr;
					asio::io_service                        dispatcher;
					std::thread                             thread;
					std::mutex                              lock;
					bool                                    finished;
			};

			ProcessorAsio(std::shared_ptr<ProcessorAsioImpl<ResourceType>> processorImplPtr)
			: processorImplPtr(processorImplPtr)
			, proxy(true) {}

			virtual HandlerWrapper wrapHandler(std::function<void()> handler, bool proxy) override
			{
				return processorImplPtr->wrapHandler(handler, proxy);
			}

		private:
			std::shared_ptr<ProcessorAsioImpl<ResourceType>> processorImplPtr;
			std::unique_ptr<ProcessorAsio<ResourceType>>     processorProxyPtr;
			bool                                             proxy;
	};

}
