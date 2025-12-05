#ifndef MAT4_H
#define MAT4_H

#include "ext/matrix_float4x4.hpp"
#include "Math/Vec3.h"
#include "Math/Vec4.h"

template <typename T>
class Mat4
{
    T m11{}, m21{}, m31{}, m41{}, m12{}, m22{}, m32{}, m42{}, m13{}, m23{}, m33{}, m43{}, m14{}, m24{}, m34{}, m44{};
    Mat4(T x0, T y0, T z0, T w0,
         T x1, T y1, T z1, T w1,
         T x2, T y2, T z2, T w2,
         T x3, T y3, T z3, T w3):

    m11(x0), m12(x1), m13(x2), m14(x3),
    m21(y0), m22(y1), m23(y2), m24(y3),
    m31(z0), m32(z1), m33(z2), m34(z3),
    m41(w0), m42(w1), m43(w2), m44(w3){}

    Mat4(T diagonalVal): m11(diagonalVal), m22(diagonalVal),
                         m33(diagonalVal), m44(diagonalVal){}

    Mat4 operator*(const Mat4& rhs)
    {
        return Mat4((m11 * rhs.m11 + m12 * rhs.m21 + m13 * rhs.m31 + m14 * rhs.m41), (m21 * rhs.m11 + m22 * rhs.m21 + m23 * rhs.m31 + m24 * rhs.m41),
                    (m31 * rhs.m11 + m32 * rhs.m21 + m33 * rhs.m31 + m34 * rhs.m41), (m41 * rhs.m11 + m42 * rhs.m21 + m43 * rhs.m31 + m44 * rhs.m41),

                    (m11 * rhs.m12 + m12 * rhs.m22 + m13 * rhs.m32 + m14 * rhs.m42), (m21 * rhs.m12 + m22 * rhs.m22 + m23 * rhs.m32 + m24 * rhs.m42),
                    (m31 * rhs.m12 + m32 * rhs.m22 + m33 * rhs.m32 + m34 * rhs.m42), (m41 * rhs.m12 + m42 * rhs.m22 + m43 * rhs.m32 + m44 * rhs.m42),

                    (m11 * rhs.m13 + m12 * rhs.m23 + m13 * rhs.m33 + m14 * rhs.m43), (m21 * rhs.m13 + m22 * rhs.m23 + m23 * rhs.m33 + m24 * rhs.m43),
                    (m31 * rhs.m13 + m32 * rhs.m23 + m33 * rhs.m33 + m34 * rhs.m43), (m41 * rhs.m13 + m42 * rhs.m23 + m43 * rhs.m33 + m44 * rhs.m43),

                    (m11 * rhs.m14 + m12 * rhs.m24 + m13 * rhs.m34 + m14 * rhs.m44), (m21 * rhs.m14 + m22 * rhs.m24 + m23 * rhs.m34 + m24 * rhs.m44),
                    (m31 * rhs.m14 + m32 * rhs.m24 + m33 * rhs.m34 + m34 * rhs.m44), (m41 * rhs.m14 + m42 * rhs.m24 + m43 * rhs.m34 + m44 * rhs.m44));
    }

    Vec4<T> operator*(const Vec4<T>& rhs)
    {
        return Vec4<T>((m11 * rhs.x + m12 * rhs.y + m13 * rhs.z + m14 * rhs.w), (m21 * rhs.x + m22 * rhs.y + m23 * rhs.z + m24 * rhs.w),
                       (m31 * rhs.x + m32 * rhs.y + m33 * rhs.z + m34 * rhs.w), (m41 * rhs.x + m42 * rhs.y + m43 * rhs.z + m44 * rhs.w));
    }

    Vec4<T> operator[](unsigned int i)
    {
	    
    }

    Mat4 getTranspose()
    {
        Mat4 temp{*this};

        temp.m21 = m12;
        temp.m31 = m13;
        temp.m41 = m14;

        temp.m12 = m21;
        temp.m32 = m23;
        temp.m42 = m24;

        temp.m13 = m31;
        temp.m23 = m32;
        temp.m43 = m34;

        temp.m14 = m41;
        temp.m24 = m42;
        temp.m34 = m43;

        return temp;
    }

    const Mat4& transpose()
    {
        Mat4 temp{*this};

        m21 = temp.m12;
        m31 = temp.m13;
        m41 = temp.m14;

        m12 = temp.m21;
        m32 = temp.m23;
        m42 = temp.m24;

        m13 = temp.m31;
        m23 = temp.m32;
        m43 = temp.m34;

        m14 = temp.m41;
        m24 = temp.m42;
        m34 = temp.m43;

        return *this;
    }

    [[nodiscard]]
	static Mat4 rotationMatrix(float angle, Vec4<T> axis)
    {
        using namespace Math;
        double cosT = std::cos(radians(angle));
        double sinT = std::sin(radians(angle));

        axis.normalize();

        return Mat4
        { (axis.x * axis.x) * (1 - cosT) + cosT,
            (axis.x * axis.y) * (1 - cosT) + axis.z * sinT,
            (axis.x * axis.z) * (1 - cosT) - axis.y * sinT,
            0

            (axis.x * axis.y) * (1 - cosT) - axis.z * sinT,
            (axis.y * axis.y) * (1 - cosT) - cosT,
            (axis.y * axis.z) * (1 - cosT) + axis.x * sinT,
            0

            (axis.x * axis.z) * (1 - cosT) + axis.y * sinT,
            (axis.y * axis.z) * (1 - cosT) - axis.x * sinT,
            (axis.z * axis.z) * (1 - cosT) + cosT,
            0,

            0, 0, 0, 1
        };
    }

    static Mat4 rotate(const Mat4& matrix, float angle, Vec4<T> axis)
    {
        return Mat4{ rotationMatrix(angle, axis) * matrix };
    }

	[[nodiscard]]
	static Mat4 scaleMatrix(float scale, Vec4<T> axis)
    {
        return Mat4{
            1 + (scale - 1) * (axis.x * axis.x),
                (scale - 1) * (axis.x * axis.y),
                (scale - 1) * (axis.x * axis.z),
            0,

                (scale - 1) * (axis.x * axis.y),
            1 + (scale - 1) * (axis.y * axis.y),
                (scale - 1) * (axis.y * axis.z),
            0,

                (scale - 1) * (axis.x * axis.z),
                (scale - 1) * (axis.y * axis.z),
            1 + (scale - 1) * (axis.z * axis.z),
            0,

            0, 0, 0, 1
        };
    }

    static Mat4 scale(const Mat4& matrix, float scale, Vec4<T> axis)
    {
        return Mat4{ scaleMatrix(scale, axis) * matrix };
    }

	[[nodiscard]]
    static Mat4 orthographicMatrix(Vec4<T> plane)
    {
        return scaleMatrix(0, plane);
    }

	[[nodiscard]]
    static Mat4 reflectMatrix(Vec4<T> axis)
    {
        return scaleMatrix(-1, axis);
    }



};
#endif //MAT4_H
