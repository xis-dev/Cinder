#pragma once


#include "CinderMath.h"
#include <cmath>
#include <iostream>


template <typename T>
requires std::is_arithmetic_v<T>
struct Vec2
{
public:
    Vec2(T val): x(val), y(val){}

    Vec2(T x_val, T y_val): x(x_val), y(y_val){}

    T x{};
    T y{};

// private:
//     double magnitude{};

public:
    [[nodiscard]]
    double magnitude() const
    {
        return  std::sqrt((x * x) + (y * y));
    }


    T operator[](size_t idx)
    {
        if (idx > 1)
        {
            std::cout << "Vector index out of range, returning last valid index.\n";
        }
        return idx == 0 ? x : y;
    }

    Vec2 operator+(const Vec2& rhs) const
    {
        return Vec2(x + rhs.x, y + rhs.y);
    }

    Vec2& operator+=(const Vec2& rhs)
    {
        x += rhs.x;
        y += rhs.y;

        return *this;
    }
    
    Vec2 operator-(const Vec2& rhs) const
    {
        return Vec2(x - rhs.x, y - rhs.y);
    }

    Vec2 operator-() const
    {
        return Vec2(-x, -y);
    }

    Vec2& operator-=(const Vec2& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;

        return *this;
    }

    Vec2 operator*(double s)
    {
        return Vec2(s * x, s * y);
    }

    Vec2& operator*=(double s)
    {
        x *= s;
        y *= s;

        return *this;
    }

    Vec2 operator/(double s)
    {
        return Vec2(x/s, y/s);
    }

    Vec2& operator/=(double s)
    {
        x /= s;
        y /= s;

        return *this;
    }

    bool operator==(const Vec2& rhs) const
    {
        return CinderMath::equals(x, rhs.x) &&
               CinderMath::equals(y, rhs.y);
    }

    bool operator!=(const Vec2& rhs) const
    {
        return !operator==(rhs);
    }


    bool operator>(const Vec2& rhs) const
    {
        return magnitude() > rhs.magnitude();
    }

    bool operator<(const Vec2& rhs) const
    {
        return magnitude() < rhs.magnitude();
    }

    [[nodiscard]]
    static T dot(const Vec2& lhs, const Vec2& rhs)
    {
        return ((lhs.x * rhs.x) + (lhs.y * rhs.y));
    }

    Vec2& normalize()
    {
        double length = magnitude();
        if (*this != zero() && length > 0.)
        {
            x = T(x/length);
            y = T(y/length);
        }
        return *this;
    }

    Vec2 getNormalized() const
    {
        Vec2 v{*this};
        double length = v.magnitude();
        if (v != zero() && length > 0.)
        {
            v.normalize();
        }
        return v;
    }

    static Vec2 zero()
    {
        return Vec2(static_cast<T>(0));
    }

    static void print(const Vec2& v, std::ostream& stream = std::cout)
    {
        stream << "Vector::(" << v.x << ", " << v.y << ")" << std::endl;
    }

};

using Vec2f = Vec2<float>;
using Vec2i = Vec2<int>;