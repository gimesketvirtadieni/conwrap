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

#include <condition_variable>
#include <queue>
#include <mutex>


namespace conwrap
{
	template<typename ResourceType1, typename Container1 = std::deque<ResourceType1>>
	class ConcurrentQueue
	{
		public:
			typedef typename Container1::iterator       iterator;
			typedef typename Container1::const_iterator const_iterator;

			ConcurrentQueue() {}

			ConcurrentQueue(const ConcurrentQueue&) = delete;

			ConcurrentQueue &operator=(const ConcurrentQueue&) = delete;

			virtual ~ConcurrentQueue() {}

			iterator begin()
			{
				return queue.c.begin();
			}

			const_iterator begin() const
			{
				return queue.c.begin();
			}

			bool empty() const
			{
				std::lock_guard<std::mutex> lock(queueLock);
				return queue.empty();
			}

			iterator end()
			{
				return queue.c.end();
			}

			const_iterator end() const
			{
				return queue.c.end();
			}

			auto front()
			{
				// making sure queue is not empty
				std::unique_lock<std::mutex> lock(queueLock);
				conditionVariable.wait(lock, [&]
				{
					return !queue.empty();
				});

				return queue.front();
			}

			void pop()
			{
				// making sure queue is not empty
				std::unique_lock<std::mutex> lock(queueLock);
				conditionVariable.wait(lock, [&]
				{
					return !queue.empty();
				});

				queue.pop();
			}

			void push(ResourceType1 item)
			{
				{
					std::lock_guard<std::mutex> lock(queueLock);
					queue.push(std::move(item));
				}
				conditionVariable.notify_all();
			}

			unsigned size() const
			{
				std::lock_guard<std::mutex> lock(queueLock);
				return queue.size();
			}

		protected:
			// defining internal struct to gain access to sequence of elements in a std::queue
			template<typename ResourceType2, typename Container2>
			struct IterableQueue : public std::queue<ResourceType2, Container2>
			{
				using std::queue<ResourceType2, Container2>::c;
			};

		private:
			IterableQueue<ResourceType1, Container1> queue;
			mutable std::mutex                       queueLock;
			std::condition_variable                  conditionVariable;
	};
}
