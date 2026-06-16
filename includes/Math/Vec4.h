#pragma once

#include <CinderMath.h>
#include "Math/Vec2.h"
#include "Math/Vec3.h"

#include "glm/vec4.hpp"

#include <iostream>
#include <cmath>


template <typename T>
requires std::is_arithmetic_v<T>
struct Vec4
{
public:
    Vec4(T val): x(val), y(val), z(val), w(val){}
    Vec4(T x_val, T y_val, T z_val, T w_val): x(x_val), y(y_val), z(z_val), w(w_val){}
    Vec4(Vec3<T> v3, T w_val): x(v3.x), y(v3.y), z(v3.z), w(w_val){}

    T x{};
    T y{};
    T z{};
    T w{};

// private:
//     double magnitude{};

public:

    operator glm::vec4()
    {
        return glm::vec4(x, y, z, w);
    }

    operator Vec3<T>()
    {
        return Vec3<T>(x, y, z);
    }

    operator Vec2<T>()
    {
        return Vec2<T>(x, y);
    }
    [[nodiscard]]
    double magnitude() const
    {
        return std::sqrt((x * x) + (y * y) + (z * z) + (w * w));
    }


    T operator[](size_t idx) const
    {
        // if (idx > 3)
        // {
        //     std::cout << "Vector index out of range, returning last valid index.\n";
        // }
        return idx == 0 ? x :
               idx == 1 ? y :
               idx == 2 ? z : w;
    }

    Vec4 operator+(const Vec4& rhs) const
    {
        return Vec4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
    }

    Vec4& operator+=(const Vec4& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;

        return *this;
    }

    Vec4 operator-(const Vec4& rhs) const
    {
        return Vec4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
    }

    Vec4 operator-() const
    {
        return Vec4(-x, -y, -z, -w);
    }

    Vec4& operator-=(const Vec4& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        w -= rhs.w;

        return *this;
    }

    Vec4 operator*(double s)
    {
        return Vec4(s * x, s * y, s * z, s * w);
    }

    Vec4& operator*=(double s)
    {
        x *= s;
        y *= s;
        z *= s;
        w *= s;

        return *this;
    }

    Vec4 operator/(double s)
    {
        return Vec4(x/s, y/s, z/s, w/s);
    }

    Vec4& operator/=(double s)
    {
        x /= s;
        y /= s;
        z /= s;
        w /= s;

        return *this;
    }

    bool operator==(const Vec4& rhs) const
    {
        return CinderMath::equals(x, rhs.x) &&
               CinderMath::equals(y, rhs.y) &&
               CinderMath::equals(z, rhs.z) &&
               CinderMath::equals(w, rhs.w);
    }

    bool operator!=(const Vec4& rhs) const
    {
        return !operator==(rhs);
    }


    bool operator>(const Vec4& rhs) const
    {
        return magnitude() > rhs.magnitude();
    }

    bool operator<(const Vec4& rhs) const
    {
        return magnitude() < rhs.magnitude();
    }

    [[nodiscard]]
    static T dot(const Vec4& lhs, const Vec4& rhs)
    {
        return ((lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z) + (lhs.w * rhs.w));
    }


    Vec4& normalize()
    {
        double length = magnitude();
        if (*this != zero() && length > 0.)
        {
            x = T(x/length);
            y = T(y/length);
            z = T(z/length);
            w = T(w/length);
        }
        return *this;
    }

    [[nodiscard]]
    Vec4 getNormalized() const
    {
        Vec4 v{*this};
        double length = v.magnitude();
        if (v != zero() && length > 0.)
        {
            v.normalize();
        }
        return v;
    }

    static Vec4 zero()
    {
        return Vec4(static_cast<T>(0));
    }

    static void print(const Vec4& v, std::ostream& stream = std::cout)
    {
        stream << "Vector::(" << v.x << ", " << v.y <<  ", " << v.z  << ", " << v.w << ")" << std::endl;
    }

};

using Vec4f = Vec4<float>;
using Vec4i = Vec4<int>;