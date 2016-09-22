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

#include <functional>


namespace conwrap
{

	// forward declaration
	template <typename ResourceType>
	class Processor;


	template <typename ResourceType>
	class HandlerContext
	{
		public:
			HandlerContext(ResourceType* r = nullptr, Processor<ResourceType>* p = nullptr)
			: resourcePtr(r)
			, processorPtr(p) {}

			virtual ~HandlerContext() {}

			inline Processor<ResourceType>* getProcessor()
			{
				return processorPtr;
			}

			inline ResourceType* getResource()
			{
				return resourcePtr;
			}

		private:
			ResourceType*            resourcePtr;
			Processor<ResourceType>* processorPtr;
	};

}
