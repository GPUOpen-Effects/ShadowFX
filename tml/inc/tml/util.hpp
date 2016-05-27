
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
* @file      util.hpp
* @brief     temporary math library utilities
* @note      this simple library will be replaced in a future version
*/

#ifndef TEMPORARY_MATH_LIB_UTIL_HPP
#define TEMPORARY_MATH_LIB_UTIL_HPP

#include <cmath>
#include <algorithm>

namespace tml
{

    inline float clamp(float x, float minVal, float maxVal)
    {
        return std::min(std::max(x, minVal), maxVal);
    }


    inline float smoothstep(float edge0, float edge1, float x)
    {
        float const tmp(clamp((x - edge0) / (edge1 - edge0), 0, 1));
        return tmp * tmp * (3 - 2 * tmp);
    }

} // namespace


#endif // include guard


