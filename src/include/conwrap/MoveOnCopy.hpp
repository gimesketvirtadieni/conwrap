/*
 * This nice helper struct is a copy-paste from Kejll's g3log (https://github.com/KjellKod/g3log/blob/master/src/g3log/moveoncopy.hpp)
 */

#pragma once

#include <utility>


namespace conwrap
{
	namespace internal
	{
		template<typename Moveable>
		struct MoveOnCopy {
			explicit MoveOnCopy(Moveable&& m)
			: moveable(std::move(m)) {}

			MoveOnCopy(const MoveOnCopy& t)
			: moveable(std::move(t.moveable)) {}

			MoveOnCopy(MoveOnCopy&& t)
			: moveable(std::move(t.moveable)) {}

			inline MoveOnCopy &operator=(const MoveOnCopy& other)
			{
				moveable = std::move(other.moveable);
				return *this;
			}

			inline MoveOnCopy &operator=(MoveOnCopy&& other)
			{
				moveable = std::move(other.moveable);
				return *this;
			}

			inline void operator()()
			{
				moveable();
			}

			inline Moveable &get()
			{
				return moveable;
			}

			inline Moveable release()
			{
				return std::move(moveable);
			}

			mutable Moveable moveable;
		};
	}
}
