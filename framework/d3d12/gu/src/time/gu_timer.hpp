
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


#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>
#include <cstdint>

namespace gu
{

    /**
    * @brief query the time elapse since the last query
    */
    class elapsed
    {
        using time_pt = std::chrono::high_resolution_clock::time_point;
        time_pt t1 = std::chrono::high_resolution_clock::now();

    public:

        /**
        * @brief get the time elapsed since the last call to this function
        * @return elapsed time in seconds
        */
        double get()
        {
            auto t2 = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> diff = t2 - t1;
            t1 = t2;
            return diff.count();
        }
    };

    /**
    * @brief timer utilities
    */
    class timer
    {
        using time_pt = std::chrono::high_resolution_clock::time_point;
        time_pt t1{};

    public:

        /**
        * @brief reset the timer
        */
        void reset()
        {
            t1 = std::chrono::high_resolution_clock::now();
        }

        /**
        * @brief get the time elapsed since a reset
        * @return elapsed time in milliseconds
        */
        std::uint64_t get_ms()
        {
            auto t2 = std::chrono::high_resolution_clock::now();
            auto diff = t2 - t1;
            return std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
        }

        /**
        * @brief get the time elapsed since a reset
        * @return elapsed time in microsecond
        */
        std::uint64_t get_us()
        {
            auto t2 = std::chrono::high_resolution_clock::now();
            auto diff = t2 - t1;
            return std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
        }
    };

}

#endif
