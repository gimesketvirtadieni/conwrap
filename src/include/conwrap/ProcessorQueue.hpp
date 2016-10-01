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
#include <conwrap/ConcurrentQueue.hpp>
#include <conwrap/HandlerContext.hpp>
#include <conwrap/HandlerWrapper.hpp>
#include <conwrap/Processor.hpp>


namespace conwrap
{

	// forward declaration
	template <typename ResourceType>
	class ProcessorAsio;


	template <typename ResourceType>
	class ProcessorQueue : public Processor<ResourceType>
	{
		// friend declaration
		template <typename> friend class ProcessorAsio;

		public:
			template <typename... Args>
			ProcessorQueue(Args... args)
			: processorImplPtr(std::make_shared<ProcessorQueueImpl<ResourceType>>(std::move(createResource(std::forward<Args>(args)...))))
			, processorProxyPtr(std::unique_ptr<ProcessorQueue<ResourceType>>(new ProcessorQueue<ResourceType>(processorImplPtr)))
			, proxy(false)
			{
				processorImplPtr->setProcessor(processorProxyPtr.get());
				processorImplPtr->start();
			}

			ProcessorQueue(std::unique_ptr<ResourceType> resource)
			: processorImplPtr(std::make_shared<ProcessorQueueImpl<ResourceType>>(std::move(resource)))
			, processorProxyPtr(std::unique_ptr<ProcessorQueue<ResourceType>>(new ProcessorQueue<ResourceType>(processorImplPtr)))
			, proxy(false)
			{
				processorImplPtr->start();
			}

			virtual ~ProcessorQueue()
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
			class ProcessorQueueImpl : public Processor<ResourceType2>
			{
				public:
					template <typename... Args>
					ProcessorQueueImpl(std::unique_ptr<ResourceType2> r)
					: resourcePtr(std::move(r)) {}

					virtual ~ProcessorQueueImpl() {}

					virtual HandlerContext<ResourceType> createHandlerContext() override
					{
						return HandlerContext<ResourceType> (getResource(), processorPtr);
					}

					virtual ResourceType2* getResource() override
					{
						return resourcePtr.get();
					}

					virtual void flush() override
					{
						// waiting for the queue to become empty
						// TODO: this should be re-designed to use proper flush semantic as current solution waits until queue becomes empty
						queue.flush();
					}

					virtual void post(HandlerWrapper handlerWrapper) override
					{
						queue.push(handlerWrapper);
					}

					void setProcessor(ProcessorQueue<ResourceType2>* p)
					{
						processorPtr = p;
					}

					void start()
					{
						{
							std::lock_guard<std::mutex> guard(lock);

							if (!thread.joinable())
							{
								thread = std::thread([&]
								{
									for(finished = false; !(queue.empty() && finished);)
									{
										// waiting for a handler
										queue.wait();

										// executing handler
										if (auto handlerWrapperPtr = queue.get())
										{
											(*handlerWrapperPtr)();

											// TODO: work in progress
											// if (!handlerWrapperPtr->getProxy())
											// {
											//  	for each in the queue
											//  		where proxy and epoche is empty
											//  		wrapped handler.epoche = current epoche
											//
											//  	increase epoche
											// }
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
								this->process([&]
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
						return wrapHandler(handler, false);
					}

					virtual HandlerWrapper wrapHandler(std::function<void()> handler, bool proxy) override
					{
						return HandlerWrapper(handler, proxy);
					}

				private:
					std::unique_ptr<ResourceType2>  resourcePtr;
					ProcessorQueue<ResourceType2>*  processorPtr;
					ConcurrentQueue<HandlerWrapper> queue;
					std::thread                     thread;
					std::mutex                      lock;
					bool                            finished;
			};

			ProcessorQueue(std::shared_ptr<ProcessorQueueImpl<ResourceType>> processorImplPtr)
			: processorImplPtr(processorImplPtr)
			, proxy(true) {}

			template <typename... Args>
			std::unique_ptr<ResourceType> createResource(Args... args)
			{
				auto resourcePtr = std::make_unique<ResourceType>(std::forward<Args>(args)...);

				// TODO: implemet compile-time reflection to make this invocation optional
				resourcePtr->setProcessor(this);

				return std::move(resourcePtr);
			}

			virtual HandlerWrapper wrapHandler(std::function<void()> handler, bool proxy) override
			{
				return processorImplPtr->wrapHandler(handler, proxy);
			}

		private:
			std::shared_ptr<ProcessorQueueImpl<ResourceType>> processorImplPtr;
			std::unique_ptr<ProcessorQueue<ResourceType>>     processorProxyPtr;
			bool                                              proxy;
	};

}
