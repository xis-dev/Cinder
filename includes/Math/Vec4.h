#pragma once
#include <iostream>
#include <stdexcept>
#include <glm/vec4.hpp>
#include <Math/Vec2.h>
#include <Math/Vec3.h>



template <typename T>
struct Vec4
{

public:
    Vec4() = default;
    T x{};
    T y{};
    T z{};
    T w{};
    Vec4(T x_val, T y_val, T z_val, T w_val): x(x_val), y(y_val), z(z_val), w(w_val){}
    Vec4(T scalar): x(scalar), y(scalar), z(scalar), w(scalar){}
    Vec4(Vec3<T> v, T w_val): x(v.x), y(v.y), z(v.z), w(w_val){}


    operator glm::vec4() const
    {
        return glm::vec4(x, y, z, w);
    }
    operator Vec3() const
    {
        return Vec3(x, y, z);
    }

    operator Vec2() const
    {
        return Vec2(x, y);
    }


    Vec4 operator+(Vec4 rhs)
    {
        return Vec4{x + rhs.x, y + rhs.y, z + rhs.z};
    }

    Vec4 operator-(Vec4 rhs)
    {
        return Vec4{x - rhs.x, y - rhs.y, y - rhs.z};
    }

    Vec4 operator*(float scalar)
    {
        return Vec4{x * scalar, y * scalar, z * scalar
        };
    }

    Vec4 operator/(float scalar)
    {
        return Vec4(x / scalar, y / scalar, z / scalar);
    }

    T operator[](int index)
    {
        return index == 1 ? x : index == 2 ? y : index == 3 ? z : throw std::out_of_range("Index out of range");
    }

    bool operator==(const Vec4& rhs)
    {
        return  Math::equals<T>(x, rhs.x) &&
				Math::equals<T>(y, rhs.y) &&
				Math::equals<T>(z, rhs.z) &&
				Math::equals<T>(w, rhs.w);

    }

    bool operator!=(const Vec4& rhs)
    {
        return !(operator==(rhs));
    }

    float getMagnitude()
    {
        if (*this == Vec4())
        {
            std::cout << "VECTOR4:: attempting to retrieve magnitude from zero vector. \n";
	        return Vec4();
        }
        return (x * x + y * y + z * z);
    }

    // Modifies the current object-
    const Vec4& normalize() const
    {
        if (*this != Vec4())
        {
            float magnitude = getMagnitude();
            x /= magnitude;
            y /= magnitude;
            z /= magnitude;
        }
        return *this;
    }

    // Returns normalized variant of the current object
    Vec4 getNormalized() const
    {
        if (*this == Vec4())
        {
	        return Vec4();
        }
        float magnitude = getMagnitude();
        return Vec4(x/magnitude, y/magnitude, z/magnitude);
    }


    static float dot(Vec4 lhs, Vec4 rhs)
    {
        return ((lhs.x * rhs.x) + (lhs.y * rhs.y), (lhs.z * rhs.z));
    }

    static Vec4 cross(Vec4 lhs, Vec4 rhs)
    {
        return Vec4(lhs.y * lhs.z, lhs.z * lhs.x, lhs.x * lhs.y);
    }

};

// Defining using directives for ease of use
using Vec4f = Vec4<float>;
using Vec4i = Vec4<int>;