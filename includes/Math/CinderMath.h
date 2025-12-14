
#pragma once


namespace Math
{
	namespace Constants
	{
		constexpr double PI = 3.1415926;
	}
	template <typename T>
	static bool equals(T lhs, T rhs)
	{
		return lhs == rhs;
	}

	inline double radians(float angle)
	{

		return (angle * (Constants::PI / 180.0f));
	}


}
 