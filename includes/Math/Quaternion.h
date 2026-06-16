#pragma once
#include <cmath>

#include "Mat4.h"
#include "Vec3.h"

// Quaternions primarily used for rotation and orientation, all unit length
class Quaternion
{
public:
    Quaternion() = default;
    Quaternion(float w_v, float x_v, float y_v, float z_v): w(w_v), x(x_v), y(y_v), z(z_v){}

    float w{};
    float x{};
    float y{};
    float z{};


    Quaternion& operator-()
    {
        w = -w;
        x = -x;
        y = -y;
        z = -z;

        return *this;
    }

    Quaternion& operator*=(const Quaternion& rhs)
    {
        Quaternion temp{*this};
        w = temp.w * rhs.w - temp.x * rhs.x - temp.y * rhs.y - temp.z * rhs.z;
        x = temp.w * rhs.x + temp.x * rhs.w + temp.y * rhs.z - temp.z * rhs.y;
        y = temp.w * rhs.y - temp.x * rhs.z + temp.y * rhs.w + temp.z * rhs.x;
        z = temp.w * rhs.z + temp.x * rhs.y - temp.y * rhs.x + temp.z * rhs.w;
        return *this;
    }

    Quaternion operator*(const Quaternion& rhs) const
    {
        Quaternion result{*this};
        result *= rhs;
        return result;
    }

    Quaternion& operator*=(float scalar)
    {
        w *= scalar;
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    Quaternion operator*(float scalar) const
    {
        Quaternion result{*this};
        result *= scalar;
        return result;
    }

    static Quaternion identity()
    {
        return {1.0f, 0.0, 0.0, 0.0};
    }
    static Quaternion slerp(Quaternion lhs, Quaternion rhs, float t)
    {
        if (t < 0.0f) return lhs;
        if (t > 1.0001f) return rhs;

        float s0;
        float s1;

        float cosAngle = dot(lhs, rhs);
        if (cosAngle < 0.0f)
        {
            rhs = -rhs;
            cosAngle = -cosAngle;
        }
        float angle = std::acos(cosAngle);

        if (cosAngle > 0.9999f)
        {
            s0 = 1.0f - t;
            s1 = t;
        }
        else
        {
            float sinAngle = sin(angle);
            float oneOverSinAngle = 1.0f / sinAngle;

            s0 = sin((1 - t) * angle) * oneOverSinAngle;
            s1 = sin(t * angle)       * oneOverSinAngle;
        }

        return    {lhs.w * s0 + rhs.w * s1,
                   lhs.x * s0 + rhs.x * s1,
                   lhs.y * s0 + rhs.y * s1,
                   lhs.z * s0 + rhs.z * s1
        };
    }
    static Quaternion conjugate(const Quaternion& q)
    {
        return {q.w, -q.x, -q.y, -q.z};
    }

    static Quaternion inverse(const Quaternion& q)
    {
        // Unit length quat, inverse = conjugate
        return conjugate(q);
    }

    static float dot(const Quaternion& lhs, const Quaternion& rhs)
    {
        return lhs.w * rhs.w + lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
    }

    // Natural log(ln)
    static Quaternion log(const Quaternion& q)
    {
        float alpha = std::acos(q.w);
        float sinAlpha = std::sin(alpha);
        if (sinAlpha < 1e-6f) return {0.0f, q.x, q.y, q.z};
        float scalar = alpha / sinAlpha;

        return {0., q.x * scalar, q.y * scalar, q.z * scalar};
    }

    static Quaternion pow(const Quaternion& q, float p)
    {
        // Does the quaternion represent any actual angular displacement
        if (fabs(q.w) < 0.9999f)
        {
            float alpha = std::acos(q.w);
            float pAlpha = p * alpha;

            float scalar = sin(pAlpha) / sin(alpha);

            return {cos(pAlpha), q.x * scalar, q.y * scalar, q.z * scalar};
        }
        return q;
    }

    static Quaternion difference(const Quaternion& lhs, const Quaternion& rhs)
    {
        return {rhs * inverse(lhs)};
    }




};
