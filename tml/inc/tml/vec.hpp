
//
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

/**
* @file      vec.hpp
* @brief     temporary math library vec definition
* @note      this simple library will be replaced in a future version
*/

#ifndef TEMPORARY_MATH_LIB_VEC_HPP
#define TEMPORARY_MATH_LIB_VEC_HPP

#include "util.hpp"

namespace tml
{

    /////////////////////////////////////////////////////////////////
    // uvec2
    /////////////////////////////////////////////////////////////////

    struct uvec2
    {
        unsigned int x;
        unsigned int y;

        uvec2() : x(0), y(0)
        {}

        explicit uvec2(unsigned int v) : x(v), y(v)
        {}

        explicit uvec2(unsigned int r, unsigned int g) : x(r), y(g)
        {}
    };

    inline bool operator == (uvec2 const& v1, uvec2 const& v2)
    {
        return v1.x == v2.x && v1.y == v2.y;
    }



    /////////////////////////////////////////////////////////////////
    // u8vec3
    /////////////////////////////////////////////////////////////////

    struct u8vec3
    {
        unsigned char x;
        unsigned char y;
        unsigned char z;

        u8vec3() : x(0), y(0), z(0)
        {}

        explicit u8vec3(unsigned char v) : x(v), y(v), z(v)
        {}

        explicit u8vec3(unsigned char r, unsigned char g, unsigned char b) : x(r), y(g), z(b)
        {}
    };



    /////////////////////////////////////////////////////////////////
    // u32vec4
    /////////////////////////////////////////////////////////////////

    struct u32vec4
    {
        unsigned int x;
        unsigned int y;
        unsigned int z;
        unsigned int w;

        u32vec4() : x(0), y(0), z(0), w(0)
        {}

        u32vec4(unsigned int v) : x(v), y(v), z(v), w(v)
        {}

        u32vec4(unsigned int r, unsigned int g, unsigned int b, unsigned int a) : x(r), y(g), z(b), w(a)
        {}

        unsigned int& operator [] (int i)
        {
            return reinterpret_cast<unsigned int*>(&x)[i];
        }

        unsigned int operator [] (int i) const
        {
            return reinterpret_cast<unsigned int const*>(&x)[i];
        }
    };

    inline u32vec4 operator + (u32vec4 const& v1, u32vec4 const& v2)
    {
        return u32vec4{ v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w };
    }

    inline u32vec4 operator / (u32vec4 const& v, unsigned int i)
    {
        return u32vec4{ v.x / i, v.y / i, v.z / i, v.w / i };
    }



    /////////////////////////////////////////////////////////////////
    // vec2
    /////////////////////////////////////////////////////////////////

    struct vec2
    {
        float x;
        float y;

        vec2() : x(0), y(0)
        {}

        explicit vec2(float v) : x(v), y(v)
        {}

        explicit vec2(float r, float g) : x(r), y(g)
        {}

        float& operator [] (int i)
        {
            return reinterpret_cast<float*>(&x)[i];
        }

        float operator [] (int i) const
        {
            return reinterpret_cast<float const*>(&x)[i];
        }
    };



    /////////////////////////////////////////////////////////////////
    // vec3
    /////////////////////////////////////////////////////////////////

    struct vec3
    {
        float x;
        float y;
        float z;

        vec3() : x(0), y(0), z(0)
        {}

        explicit vec3(float v) : x(v), y(v), z(v)
        {}

        explicit vec3(float r, float g, float b) : x(r), y(g), z(b)
        {}

        float& operator [] (int i)
        {
            return reinterpret_cast<float*>(&x)[i];
        }

        float operator [] (int i) const
        {
            return reinterpret_cast<float const*>(&x)[i];
        }
    };

    inline vec3 operator * (vec3 const& v, float f)
    {
        return vec3{ v.x * f, v.y * f, v.z * f };
    }

    inline vec3 operator * (float f, vec3 const& v)
    {
        return v * f;
    }

    inline vec3 operator * (vec3 const& v1, vec3 const& v2)
    {
        return vec3{ v1.x * v2.x, v1.y * v2.y, v1.z * v2.z };
    }

    inline vec3 operator + (vec3 const& v1, vec3 const& v2)
    {
        return vec3{ v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
    }

    inline vec3 operator - (vec3 const& v1, vec3 const& v2)
    {
        return vec3{ v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
    }

    inline vec3 operator - (vec3 const& v)
    {
        return vec3{ -v.x , -v.y, -v.z };
    }

    inline vec3 operator / (vec3 const& v, float f)
    {
        return vec3{ v.x / f, v.y / f, v.z / f };
    }

    inline vec3 cross(vec3 const& x, vec3 const& y)
    {
        return vec3(
            x.y * y.z - y.y * x.z,
            x.z * y.x - y.z * x.x,
            x.x * y.y - y.x * x.y);
    }

    inline float dot(vec3 const& v1, vec3 const& v2)
    {
        return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
    }

    inline vec3 normalize(vec3 const& v)
    {
        return v / std::sqrt(dot(v, v));
    }



    /////////////////////////////////////////////////////////////////
    // vec4
    /////////////////////////////////////////////////////////////////

    struct vec4
    {
        float x;
        float y;
        float z;
        float w;

        vec4() : x(0), y(0), z(0), w(0)
        {}

        vec4(float v) : x(v), y(v), z(v), w(v)
        {}

        vec4(float r, float g, float b, float a) : x(r), y(g), z(b), w(a)
        {}

        vec4(vec3 const& v, float f) : x(v.x), y(v.y), z(v.z), w(f)
        {
        }

        float& operator [] (int i)
        {
            return reinterpret_cast<float*>(&x)[i];
        }

        float operator [] (int i) const
        {
            return reinterpret_cast<float const*>(&x)[i];
        }

        explicit operator vec3()
        {
            return vec3{ x, y, z };
        }
    };

    inline vec4 operator * (vec4 const& v, float f)
    {
        return vec4{ v.x * f, v.y * f, v.z * f, v.w * f };
    }

    inline vec4 operator * (float f, vec4 const& v)
    {
        return v * f;
    }

    inline vec4 operator * (vec4 const& v1, vec4 const& v2)
    {
        return vec4{ v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w };
    }

    inline vec4 operator + (vec4 const& v1, vec4 const& v2)
    {
        return vec4{ v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w };
    }

    inline vec4 operator / (vec4 const& v, float f)
    {
        return vec4{ v.x / f, v.y / f, v.z / f, v.w / f };
    }

    inline vec4 operator - (vec4 const& v1, vec4 const& v2)
    {
        return vec4{ v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w };
    }


} // namespace


#endif // include guard


