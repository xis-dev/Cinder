#pragma once
#include <cmath>
#include <glm/vec3.hpp>
#include <stdexcept>
#include <iostream>


#include "Math.h"
#include "Vec2.h"


template <typename T>
struct Vec3
{

public:
    Vec3() = default;
    T x{};
    T y{};
    T z{};
    Vec3(T x_val, T y_val, T z_val): x(x_val), y(y_val), z(z_val){}
    Vec3(T scalar): x(scalar), y(scalar), z(scalar){}

     operator glm::vec3() const
    {
        return glm::vec3(x, y, z);
    }

    operator Vec2<T>() const
    {
        return Vec2<T>(x, y);
    }
    Vec3 operator+(Vec3 rhs)
    {
        return Vec3{x + rhs.x, y + rhs.y, z + rhs.z};
    }

    Vec3 operator-(Vec3 rhs)
    {
        return Vec3{x - rhs.x, y - rhs.y, z - rhs.z};
    }

    Vec3 operator*(float scalar)
    {
        return Vec3{x * scalar, y * scalar, z * scalar
        };
    }

    Vec3 operator/(float scalar)
    {
        return Vec3(x / scalar, y / scalar, z / scalar);
    }

    bool operator==(const Vec3& rhs)
    {
        return Math::equals<T>(x, rhs.x) &&
        	   Math::equals<T>(y, rhs.y) &&
               Math::equals<T>(z, rhs.z);
    }

    bool operator!=(const Vec3& rhs)
    {
        return !(operator==(rhs));
    }

    T operator[](int index)
    {
        // Returns z for positive index values
        return index == 0 ? x : index == 1 ? y : index == 2 ? z : throw std::out_of_range("Index out of range");
    }

    static void print(const Vec3& v, std::ostream& out = std::cout)
    {
        out << "x: " << v.x << ", y: " << v.y << ", z: " << v.z << std::endl;
    }

    float getMagnitude()
    {
        return std::sqrt(static_cast<float>((x * x + y * y + z * z)));
    }

    // Modifies the current object-
    const Vec3& normalize()
    {
        if (*this != zero())
        {
            float magnitude = getMagnitude();
            x /= magnitude;
            y /= magnitude;
            z /= magnitude;
        }
        return *this;
    }

    // Returns normalized variant of the current object
    Vec3 getNormalized()
    {
        if (*this == zero()) return zero();
        float magnitude = (getMagnitude());
        return Vec3(x/magnitude, y/magnitude, z/magnitude);
    }


    static float dot(Vec3 lhs, Vec3 rhs)
    {
        return ((lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z));
    }

    static Vec3 cross(Vec3 lhs, Vec3 rhs)
    {
        return Vec3(lhs.y * lhs.z, lhs.z * lhs.x, lhs.x * lhs.y);
    }

    static Vec3 zero()
    {
        return Vec3();
    }
};

// Defining using directives for ease of use
using Vec3f = Vec3<float>;
using Vec3i = Vec3<int>;