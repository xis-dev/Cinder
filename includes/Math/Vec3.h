#pragma once

#include "Math/Vec2.h"
#include <CinderMath.h>

#include "glm/vec3.hpp"

#include <iostream>
#include <cmath>


template <typename T>
requires std::is_arithmetic_v<T>
struct Vec3
{
public:
    Vec3(T val): x(val), y(val), z(val){}

    Vec3(T x_val, T y_val, T z_val): x(x_val), y(y_val), z(z_val){}

    T x{};
    T y{};
    T z{};

// private:
//     double magnitude{};

public:

    operator glm::vec3()
    {
        return glm::vec3(x, y, z);
    }
    operator Vec2<T>()
    {
        return Vec2<T>(x, y);
    }

    [[nodiscard]]
    double magnitude() const
    {
        return  std::sqrt((x * x) + (y * y) + (z * z));
    }


    T operator[](size_t idx)
    {
        if (idx > 2)
        {
            std::cout << "Vector index out of range, returning last valid index.\n";
        }
        return idx == 0 ? x : idx == 1 ? y : z;
    }

    Vec3 operator+(const Vec3& rhs) const
    {
        return Vec3(x + rhs.x, y + rhs.y, z + rhs.z);
    }

    Vec3& operator+=(const Vec3& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;

        return *this;
    }

    Vec3 operator-(const Vec3& rhs) const
    {
        return Vec3(x - rhs.x, y - rhs.y, z - rhs.z);
    }

    Vec3 operator-() const
    {
        return Vec3(-x, -y, -z);
    }

    Vec3& operator-=(const Vec3& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;

        return *this;
    }

    Vec3 operator*(double s)
    {
        return Vec3(s * x, s * y, s * z);
    }

    Vec3& operator*=(double s)
    {
        x *= s;
        y *= s;
        z *= s;

        return *this;
    }

    Vec3 operator/(double s)
    {
        return Vec3(x/s, y/s, z/s);
    }

    Vec3& operator/=(double s)
    {
        x /= s;
        y /= s;
        z /= s;

        return *this;
    }

    bool operator==(const Vec3& rhs) const
    {
        return CinderMath::equals(x, rhs.x) &&
               CinderMath::equals(y, rhs.y) &&
               CinderMath::equals(z, rhs.z);
    }

    bool operator!=(const Vec3& rhs) const
    {
        return !operator==(rhs);
    }


    bool operator>(const Vec3& rhs) const
    {
        return magnitude() > rhs.magnitude();
    }

    bool operator<(const Vec3& rhs) const
    {
        return magnitude() < rhs.magnitude();
    }

    [[nodiscard]]
    static T dot(const Vec3& lhs, const Vec3& rhs)
    {
        return ((lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z));
    }

    static Vec3 cross(const Vec3& lhs, const Vec3& rhs)
    {
        return Vec3(lhs.y * rhs.z - lhs.z * rhs.y,
                    lhs.z * rhs.x - lhs.x * rhs.z,
                    lhs.x * rhs.y - lhs.y * rhs.x);
    }

    Vec3& normalize()
    {
        double length = magnitude();
        if (*this != zero() && length > 0.)
        {
            x = T(x/length);
            y = T(y/length);
            z = T(z/length);
        }
        return *this;
    }

    [[nodiscard]]
    Vec3 getNormalized() const
    {
        Vec3 v{*this};
        double length = v.magnitude();
        if (v != zero() && length > 0.)
        {
            v.normalize();
        }
        return v;
    }

    static Vec3 zero()
    {
        return Vec3(static_cast<T>(0));
    }

    static void print(const Vec3& v, std::ostream& stream = std::cout)
    {
        stream << "Vector::(" << v.x << ", " << v.y <<  ", " << v.z  << ")" << std::endl;
    }

};

using Vec3f = Vec3<float>;
using Vec3i = Vec3<int>;