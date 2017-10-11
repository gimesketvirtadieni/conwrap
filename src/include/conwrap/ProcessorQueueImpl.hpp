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
#include <conwrap/EpochGenerator.hpp>
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
				, processorPtr(nullptr)
				, processorProxyPtr(nullptr)
				, finished(false)
				, epoch(0) {}

				virtual ~ProcessorQueueImpl() {}

				virtual void flush() override
				{
					// figuring out current epoch value
					auto currentEpoch = this->process([&]() -> auto
					{
						// it is safe to copy current epoch value here as it will be done on the processor's thread
						return epochGenerator.current();
					}).get();

					// waiting for all 'child' tasks to complete
					while (childExists(currentEpoch))
					{
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
						if (h.getProxy() && Epoch(1) <= epoch && epoch <= currentEpoch)
						{
							found = true;
							break;
						}
					}
					return found;
				}

				virtual Processor<ResourceType>* getProcessor() override
				{
					return processorPtr;
				}

				virtual ProcessorProxy<ResourceType>* getProcessorProxy() override
				{
					return processorProxyPtr;
				}

				virtual void post(TaskWrapped task) override
				{
					queue.push(std::move(task));
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
									// waiting for a task to arrive
									TaskWrapped& task = queue.front();

									// setting current epoch to be used for submitted tasks via processor proxy
									epoch = task.getEpoch();

									// executing task
									task();

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

							// waiting until consumer finsihes all pending tasks
							thread.join();
						}
					}
				}

				virtual TaskWrapped wrap(std::function<void()> task) override
				{
					return std::move(wrap(std::move(task), false));
				}

				virtual TaskWrapped wrap(std::function<void()> task, bool proxy) override
				{
					// it is safe to use this->epoch here as submitting 'child' tasks can be done only from processor's thread
					return std::move(TaskWrapped(std::move(task), proxy, (proxy ? epoch : epochGenerator.generate())));
				}

			private:
				std::unique_ptr<ResourceType> resourcePtr;
				Processor<ResourceType>*      processorPtr;
				ProcessorProxy<ResourceType>* processorProxyPtr;
				ConcurrentQueue<TaskWrapped>  queue;
				std::thread                   thread;
				std::mutex                    threadLock;
				bool                          finished;
				EpochGenerator                epochGenerator;
				Epoch                         epoch;
		};
	}
}
