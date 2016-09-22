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
#include <exception>
#include <queue>
#include <mutex>


template<typename T1, typename Container1 = std::deque<T1>>
class ConcurrentQueue
{
	public:
		typedef typename Container1::iterator       iterator;
		typedef typename Container1::const_iterator const_iterator;

		ConcurrentQueue() {}

		virtual ~ConcurrentQueue() {}

		iterator begin()
		{
			return queue_.c.begin();
		}

		const_iterator begin() const
		{
			return queue_.c.begin();
		}

		bool empty() const
		{
			std::lock_guard<std::mutex> lock(m_);
			return queue_.empty();
		}

		iterator end()
		{
			return queue_.c.end();
		}

		const_iterator end() const
		{
			return queue_.c.end();
		}

		void flush()
		{
			{
				std::unique_lock<std::mutex> lock(m_);
				data_cond_.wait(lock, [&] { return queue_.empty();});
			}
		}

		T1* get()
		{
			T1* result = nullptr;
			{
				std::lock_guard<std::mutex> lock(m_);
				if (!queue_.empty()) {
					result = &queue_.front();
				}
			}
			return result;
		}

		void push(T1 item)
		{
			{
				std::lock_guard<std::mutex> lock(m_);
				queue_.push(std::move(item));
			}
			data_cond_.notify_all();
		}

		bool remove() {
			auto result = false;
			{
				std::lock_guard<std::mutex> lock(m_);
				if (!queue_.empty()) {

					// remove the first item in the queue
					queue_.pop();
					data_cond_.notify_all();
					result = true;
				}
			}
			return result;
		}

		unsigned size() const
		{
			std::lock_guard<std::mutex> lock(m_);
			return queue_.size();
		}

		void wait()
		{
			{
				std::unique_lock<std::mutex> lock(m_);
				data_cond_.wait(lock, [&] {return !queue_.empty();});
			}
		}

	protected:
		// defining internal struct to gain access to sequence of elements in a std::queue
		template<typename T2, typename Container2>
		struct IterableQueue : public std::queue<T2, Container2>
		{
			using std::queue<T2, Container2>::c;
		};


	private:
		IterableQueue<T1, Container1> queue_;
		mutable std::mutex m_;
		std::condition_variable data_cond_;

		ConcurrentQueue(const ConcurrentQueue &other) = delete;
		ConcurrentQueue &operator=(const ConcurrentQueue &) = delete;
};
