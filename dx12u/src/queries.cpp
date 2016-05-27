
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
* @file      queries.cpp
* @brief     dx12 query helpers implemetation
*/

#include <queries.hpp>

namespace dx12u
{

    timestamp_query::timestamp_query(device const& dvc, dx12u::cmd_queue const& q, int gpu_ordinal, std::size_t max_num_query) : num_query(max_num_query)
    {
        if (!dvc.Get())
        {
            throw error{ "null device" };
        }

        std::uint32_t gpu_mask = gpu_ordinal < 0 ? 1 : (1u << static_cast<std::uint32_t>(gpu_ordinal));

        // alloc query buffer
        size_t buf_aligned_sz = (num_query * sizeof(std::uint64_t) + 4095) & ~4095; // page aligned
        auto heap_read_back = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK, gpu_mask, gpu_mask);
        auto buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(buf_aligned_sz);
        auto r = dvc->CreateCommittedResource(&heap_read_back, D3D12_HEAP_FLAG_NONE,
            &buffer_desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&buffer));
        dx12u::throw_if_error(r, "timestamp resource creation failure");

        // query heap
        D3D12_QUERY_HEAP_DESC heap_desc{};
        heap_desc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
        heap_desc.Count = static_cast<std::uint32_t>(num_query);
        heap_desc.NodeMask = gpu_mask;
        r = dvc->CreateQueryHeap(&heap_desc, IID_PPV_ARGS(&qh));
        dx12u::throw_if_error(r, "timestamp query heap creation failure");

        // clock frequency
        dvc->SetStablePowerState(true);
        q.get_com_ptr()->GetTimestampFrequency(&clock_freq);
    }

    std::uint64_t timestamp_query::get(std::size_t idx) const
    {
        std::uint64_t* buffer_data = nullptr;
        D3D12_RANGE range = { idx * sizeof(std::uint64_t), (idx + 1) * sizeof(std::uint64_t) }; // debug layer gives a wraning if this is not used
        // map query buffer
        auto r = buffer->Map(0, &range, reinterpret_cast<void**>(&buffer_data));
        dx12u::throw_if_error(r, "timestamp resource mapping failure");
        assert(!!buffer_data);

        auto ts = buffer_data[idx];

        buffer->Unmap(0, nullptr);

        return ts;
    }

} // namespace dx12u


