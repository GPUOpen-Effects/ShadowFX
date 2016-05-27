
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
* @file      queries.hpp
* @brief     dx12 query helpers
*/

#ifndef DX12UTIL_QUERIES_HPP
#define DX12UTIL_QUERIES_HPP

#include <cmd_mgr.hpp>
#include <memory.hpp>

namespace dx12u
{

    /**
    * @brief query heap type
    */
    using query_heap = com_ptr<ID3D12QueryHeap>;

    /**
    * @brief a gpu timestamp query object
    */
    class timestamp_query
    {
        std::size_t num_query = 0; // max number of queries
        resource buffer = nullptr; // buffer that holds the result
        query_heap qh = nullptr;
        std::size_t max_idx = 0;
        std::uint64_t clock_freq = 0;

    public:

        timestamp_query(const timestamp_query&) = delete;
        timestamp_query& operator = (const timestamp_query&) = delete;

        /**
        * @brief create an invalid timestamp query object
        */
        timestamp_query()
        {}

        /**
        * @brief create a timestamp query
        * @param dvc device
        * @param q command queue
        * @param gpu_ordinal gpu on which to execture the queries
        * @param max_num_query maximum number of queries to sample
        */
        timestamp_query(device const& dvc, dx12u::cmd_queue const& q, int gpu_ordinal = -1, std::size_t max_num_query = 512);

        /**
        * @brief sample the timestamp
        * @param cl command list
        * @param idx query index
        * @note cl must be created with the same gpu_ordinal as this query
        * @note UB if idx is not less than get_max_num_query()
        * @note UB if idx is used more than once before calling fetch
        */
        void sample(gfx_cmd_list& cl, std::size_t idx)
        {
            assert(cl.Get() != nullptr && idx < num_query);
            cl->EndQuery(qh.Get(), D3D12_QUERY_TYPE_TIMESTAMP, static_cast<std::uint32_t>(idx));
            max_idx = std::max(max_idx, idx);
        }

        /**
        * @brief read back query data
        * @param cl command list
        * @param num number of queries to read back. if 0 all queries are read back
        */
        void fetch(gfx_cmd_list& cl, std::size_t num = 0) const
        {
            num = num == 0 ? max_idx + 1 : num;
            assert(cl.Get() != nullptr && num < num_query);
            cl->ResolveQueryData(qh.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0, static_cast<std::uint32_t>(num), buffer.Get(), 0);
        }

        std::size_t get_max_num_query() const noexcept
        {
            return num_query;
        }

        /**
        * @brief get the timestamp at index idx
        * @param idx query index
        * @return timestamp (elapsed time in seconds)
        */
        float get_s(std::size_t idx) const
        {
            assert(clock_freq > 0);
            return static_cast<float>(static_cast<double>(get(idx)) / clock_freq);
        }

        /**
        * @brief get the timestamp at index idx
        * @param idx query index
        * @return timestamp
        */
        std::uint64_t get(std::size_t idx) const;

        /**
        * @brief get clock frequency
        * @param idx query index
        * @return timestamp clock frequency
        */
        std::uint64_t get_freq() const noexcept
        {
            return clock_freq;
        }

        /**
        * @brief check whether this query is valid
        * @param idx query index
        * @return timestamp
        */
        bool valid() const noexcept
        {
            return qh.Get() != nullptr;
        }
    };

} // namespace dx12u



#endif // DX12UTIL_QUERIES_HPP
