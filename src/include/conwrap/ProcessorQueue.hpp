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
			: processorImplPtr(std::make_shared<ProcessorQueueImpl<ResourceType>>(std::move(std::make_unique<ResourceType>(std::forward<Args>(args)...))))
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
				processorImplPtr->setProcessor(processorProxyPtr.get());
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
					: resourcePtr(std::move(r))
					, epoch(1) {}

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
						// figuring out current epoch
						auto currentEpoch = this->process([=]() -> auto
						{
							return this->epoch;
						}).get();

						// waiting for any 'child' handlers to be processed
						while (childExists(currentEpoch))
						{
							// TODO: insert flush handler after the last child instead of adding at the end of the queue
							this->process([=] {}).wait();
						}
					}

					virtual void post(HandlerWrapper handlerWrapper) override
					{
						queue.push(handlerWrapper);
					}

					void setProcessor(ProcessorQueue<ResourceType2>* p)
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

				private:
					std::unique_ptr<ResourceType2>  resourcePtr;
					ProcessorQueue<ResourceType2>*  processorPtr;
					ConcurrentQueue<HandlerWrapper> queue;
					std::thread                     thread;
					std::mutex                      lock;
					bool                            finished;
					unsigned long long              epoch;
			};

			ProcessorQueue(std::shared_ptr<ProcessorQueueImpl<ResourceType>> processorImplPtr)
			: processorImplPtr(processorImplPtr)
			, proxy(true) {}

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
