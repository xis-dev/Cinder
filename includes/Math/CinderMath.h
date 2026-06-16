
#pragma once


#include <cmath>
#include <numbers>
#include <ranges>
#include <limits>

namespace CinderMath
{
	namespace Constants
	{
		constexpr double PI = std::numbers::pi;
	}

	template <typename T>
	static bool equals(T lhs, T rhs)
	{
		if constexpr(std::is_floating_point_v<T>)
		{
			constexpr auto eps = std::numeric_limits<T>::epsilon();
			return std::abs(lhs - rhs) <= eps * std::max(T(1),
												 std::max(std::abs(lhs), std::abs(rhs)));
		}

		return lhs == rhs;
	}

	inline double radians(float angle)
	{
		return (angle * (Constants::PI / 180.0f));
	}

}
 