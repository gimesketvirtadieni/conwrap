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
#include <conwrap/Epoch.hpp>
#include <conwrap/Processor.hpp>
#include <conwrap/TaskResult.hpp>
#include <conwrap/TaskResultProxy.hpp>
#include <conwrap/TaskWrapped.hpp>
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
			template <typename> friend class ProcessorQueue;
			template <typename> friend class ProcessorQueueProxy;

			public:
				ProcessorQueueImpl(std::unique_ptr<ResourceType> r)
				: resourcePtr(std::move(r))
				, nextEpoch(1)
				, currentEpoch(0) {}

				virtual ~ProcessorQueueImpl() {}

				virtual void flush() override
				{
					// figuring out next epoch value
					auto currentEpoch = this->process([&]() -> auto
					{
						return getNextEpoch();
					}).get();

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
				bool childExists(Epoch currentEpoch)
				{
					auto found = false;

					for (auto& h : queue)
					{
						auto epoch = h.getEpoch();
						if (h.getProxy() && 0 < epoch && epoch < currentEpoch)
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

				virtual Processor<ResourceType>* getProcessor() override
				{
					return processorPtr;
				}

				virtual ProcessorProxy<ResourceType>* getProcessorProxy() override
				{
					return processorProxyPtr;
				}

				virtual void post(TaskWrapped handlerWrapper) override
				{
					queue.push(std::move(handlerWrapper));
				}

				inline void setProcessor(Processor<ResourceType>* p)
				{
					processorPtr = p;
				}

				inline void setProcessorProxy(ProcessorProxy<ResourceType>* pp)
				{
					processorProxyPtr = pp;
				}

				void start()
				{
					{
						std::lock_guard<std::mutex> guard(threadLock);

						if (!thread.joinable())
						{
							thread = std::thread([&]
							{
								for(finished = false; !(queue.empty() && finished);)
								{
									// waiting for a handler
									TaskWrapped& handler = queue.front();

									// setting current epoch to be used for submitted tasks via processor proxy
									currentEpoch = handler.getEpoch();

									// executing handler
									handler();

									// removing executed item
									queue.pop();
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
							this->process([&]
							{
								finished = true;
							});

							// waiting until consumer finsihes all pending handlers
							thread.join();
						}
					}
				}

				virtual TaskWrapped wrap(std::function<void()> handler) override
				{
					return std::move(wrap(std::move(handler), false));
				}

				virtual TaskWrapped wrap(std::function<void()> handler, bool proxy) override
				{
					return std::move(TaskWrapped(std::move(handler), proxy, (proxy ? currentEpoch : nextEpoch++)));
				}

			private:
				std::unique_ptr<ResourceType> resourcePtr;
				Processor<ResourceType>*      processorPtr;
				ProcessorProxy<ResourceType>* processorProxyPtr;
				ConcurrentQueue<TaskWrapped>  queue;
				std::thread                   thread;
				std::mutex                    threadLock;
				bool                          finished;
				Epoch                         nextEpoch;
				Epoch                         currentEpoch;
		};
	}
}
