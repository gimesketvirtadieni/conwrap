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

#include <thread>

#include "ConcurrentQueue.hpp"
#include "Processor.hpp"


template <typename ResourceType>
class QueueProcessor : public Processor<ResourceType>
{
	public:
		template <typename... Args>
		QueueProcessor(Args... args) :
			proxy(false)
			//processorProxy(QueueProcessor<ResourceType>(this))
		{

			// creating an instance of the resource object and passing ownership to ProcessorImpl
			// TODO: figure out possibility to avoid using unique_ptr
			//processorImplPtr->setResource(std::move(std::make_unique<ResourceType>(processorProxyPtr.get(), std::forward<Args>(args)...)));

			//LOG(DEBUG) << "CLI: Processor object was created (id=" << this << ", thread=" << processorImplPtr->getThread()->get_id() << ")";
		}

		virtual ~QueueProcessor()
		{
		}
/*
		inline boost::asio::io_service* getDispatcher() const {
			return processorImplPtr->getDispatcher();
		}

		inline ResourceType* getResource() const {
			return processorImplPtr->getResource();
		}

		inline void flush() {
			processorImplPtr->flush();
		}

		template <typename F>
		inline auto process(F fun) const -> std::future<decltype(fun(getResource()))> {
			return processorImplPtr->process(fun);
		}
*/
	protected:
		QueueProcessor(QueueProcessor<ResourceType>* processorPtr) :
			//processorPtr(processorPtr),
			proxy(true)
		{
			//LOG(DEBUG) << "CLI: Processor" << (proxy ? " (proxy)" : "" ) << " object was created (id=" << this << ", thread=" << processorImplPtr->getThread()->get_id() << ")";
		}

	private:
		const bool proxy;
		QueueProcessor<ResourceType> processorPtr;
		//std::shared_ptr<ProcessorImpl<std::thread, boost::asio::io_service, ResourceType>> processorImplPtr;
};

/*
template<typename ResourceType>
inline bool operator==(const Processor<ResourceType>& p, const ResourceType& r) noexcept {
	return p.getResource() == &r;
}


template<typename ResourceType>
inline bool operator==(const ResourceType& r, const Processor<ResourceType>& p) noexcept {
	return &r == p.getResource();
}
*/
