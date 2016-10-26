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

#include <conwrap/ProcessorProxy.hpp>


namespace conwrap
{
	template <typename ResourceType, template<typename ResourceType, typename ResultType> class TaskType>
	class Processor : public ProcessorProxy<ResourceType, TaskType>
	{
		public:
			virtual void flush() = 0;
	};
}
