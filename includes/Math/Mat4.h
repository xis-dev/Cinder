#pragma once

#include "Vec3.h"
#include "Vec4.h"

#include "glm/mat4x4.hpp"
#include <cmath>
#include <concepts>


template <typename T>
requires std::is_arithmetic_v<T>
// Column major-order matrix
class Mat4
{
public:
    Mat4() = default;
    Mat4(T scalar, bool fill = false)
    {
        if (!fill)
        {
            cols[0].x = scalar;
            cols[1].y = scalar;
            cols[2].z = scalar;
            cols[3].w = scalar;
            return;
        }

        cols[0] = cols[1] = cols[2] = cols[3] = Vec4<T>(scalar);
    }

    Mat4(T x0, T y0, T z0, T w0,
     T x1, T y1, T z1, T w1,
     T x2, T y2, T z2, T w2,
     T x3, T y3, T z3, T w3)
    {
        cols[0].x = x0; cols[0].y = y0; cols[0].z = z0; cols[0].w = w0;
        cols[1].x = x1; cols[1].y = y1; cols[1].z = z1; cols[1].w = w1;
        cols[2].x = x2; cols[2].y = y2; cols[2].z = z2; cols[2].w = w2;
        cols[3].x = x3; cols[3].y = y3; cols[3].z = z3; cols[3].w = w3;
    }

    Mat4(Vec4<T> v0, Vec4<T> v1,
         Vec4<T> v2, Vec4<T> v3): cols{v0, v1, v2, v3} {}

    Vec4<T> cols[4]{Vec4<T>(0), Vec4<T>(0), Vec4<T>(0), Vec4<T>(0)};

    operator glm::mat4()
    {
       return {glm::vec4(cols[0]), glm::vec4(cols[1]), glm::vec4(cols[2]), glm::vec4(cols[3])};
    }



    Mat4& operator+=(const Mat4& rhs)
    {
        for (size_t i = 0; i < 4; ++i)
        {
           cols[i] += rhs.cols[i];
        }
        return *this;
    }

    Mat4 operator+(const Mat4& rhs) const
    {
        Mat4 result{*this};
        result += rhs;
        return result;
    }

    Mat4& operator-=(const Mat4& rhs)
    {
        for (size_t i = 0; i < 4; ++i)
        {
            cols[i] -= rhs.cols[i];
        }
        return *this;
    }

    Mat4 operator-(const Mat4& rhs) const
    {
        Mat4 result{*this};
        result -= rhs;
        return result;
    }

    Mat4& operator*=(const Mat4& rhs)
    {
        const Mat4 temp{*this};
        for (size_t i = 0; i < 4; ++i)
        {
            cols[i] = temp.multiply(rhs.cols[i]);
        }
        return *this;
    }

    Mat4 operator*(const Mat4& rhs) const
    {
        Mat4 result{*this};
        result *= rhs;
        return result;
    }

    Mat4 getTranspose()
    {
        return Mat4{cols[0].x, cols[1].x, cols[2].x, cols[3].x, // C0
                    cols[0].y, cols[1].y, cols[2].y, cols[3].y, // C1
                    cols[0].z, cols[1].z, cols[2].z, cols[3].z, // C2
                    cols[0].w, cols[1].w, cols[2].w, cols[3].w  // C3
        };
    }
    Vec4<T> multiply(const Vec4<T>& v) const
    {
        return Vec4<T>{
            cols[0].x * v.x + cols[1].x * v.y + cols[2].x * v.z + cols[3].x * v.w,
            cols[0].y * v.x + cols[1].y * v.y + cols[2].y * v.z + cols[3].y * v.w,
            cols[0].z * v.x + cols[1].z * v.y + cols[2].z * v.z + cols[3].z * v.w,
            cols[0].w * v.x + cols[1].w * v.y + cols[2].w * v.z + cols[3].w * v.w
        };
    }

    // Angle in radians
    static Mat4 rotate(const Mat4& mat, const double angle, Vec3<T> axis)
    {
        axis.normalize();
        return Mat4{Vec4<T>((axis.x * axis.x) * (1 - cos(angle)) + cos(angle), // C0
                             (axis.x * axis.y) * (1 - cos(angle)) + axis.z * sin(angle),
                             (axis.x * axis.z) * (1 - cos(angle)) - axis.y * sin(angle), 0.0),

                     Vec4<T>((axis.x * axis.y) * (1 - cos(angle)) - axis.z * sin(angle), // C1
                             (axis.y * axis.y) * (1 - cos(angle)) + cos(angle),
                             (axis.y * axis.z) * (1 - cos(angle)) + axis.x * sin(angle), 0.0),

                     Vec4<T>((axis.x * axis.z) * (1 - cos(angle)) + axis.y * sin(angle), // C2
                             (axis.y * axis.z) * (1 - cos(angle)) - axis.x * sin(angle),
                             (axis.z * axis.z) * (1 - cos(angle)) + cos(angle), 0.0),

                    Vec4<T>(0, 0, 0, 1) //C3
        } * mat;
    }


    static Mat4 translate(Mat4 mat, const Vec3<T>& translation)
    {
        mat.cols[3].x += translation.x;
        mat.cols[3].y += translation.y;
        mat.cols[3].z += translation.z;

        return mat;
    }

    static Mat4 scale(Mat4 mat, const Vec3<T>& s)
    {
        mat.cols[0].x *= s.x;
        mat.cols[1].y *= s.y;
        mat.cols[2].z *= s.z;

        return mat;
    }

    static Mat4 scale(const Mat4& mat, float s, const Vec3<T>& axis)
    {
        return Mat4{Vec4<T>(1 + (s - 1) * (axis.x * axis.x),
                                (s - 1) * (axis.x * axis.y),
                                (s - 1) * (axis.x * axis.z), 0.0),

                    Vec4<T>(    (s - 1) * (axis.x * axis.y),
                            1 + (s - 1) * (axis.y * axis.y),
                                (s - 1) * (axis.y * axis.z), 0.0),

                    Vec4<T>(    (s - 1) * (axis.x * axis.z),
                                (s - 1) * (axis.y * axis.z),
                            1 + (s - 1) * (axis.z * axis.z), 0.0),

                    Vec4<T>(0, 0, 0, 1)
        } * mat;
    }



    static void print(const Mat4& mat, std::ostream& stream = std::cout)
    {
        stream << "Matrix4x4::(\nColumn0(" << mat.cols[0].x << ", " << mat.cols[0].y << ", " << mat.cols[0].z << ", " << mat.cols[0].w << "),\n" <<
                                "Column1(" << mat.cols[1].x << ", " << mat.cols[1].y << ", " << mat.cols[1].z << ", " << mat.cols[1].w << "),\n" <<
                                "Column2(" << mat.cols[2].x << ", " << mat.cols[2].y << ", " << mat.cols[2].z << ", " << mat.cols[2].w << "),\n" <<
                                "Column3(" << mat.cols[3].x << ", " << mat.cols[3].y << ", " << mat.cols[3].z << ", " << mat.cols[3].w << "))\n";
    }

};

using Mat4f = Mat4<float>;
using Mat4i = Mat4<int>;