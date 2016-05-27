
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
* @file      descriptor_heap.hpp
* @brief     dx12 descriptor heap implementation
*/

#include <descriptor_heap.hpp>

using std::uint32_t;

namespace dx12u
{
    descriptor_heap::descriptor_heap(ID3D12Device* dev, D3D12_DESCRIPTOR_HEAP_TYPE type, std::size_t n, int gpu_ordinal)
        : num_slot(n)
    {
        if (dev == nullptr)
        {
            throw error{ "descriptor heap creation failed. null device" };
        }

        if (n == 0)
        {
            throw error{ "descriptor heap creation failed. zero slots" };
        }

        std::uint32_t gpu_mask = gpu_ordinal < 0 ? 1 : (1u << static_cast<std::uint32_t>(gpu_ordinal));

        D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
        heap_desc.NumDescriptors = static_cast<uint32_t>(n);
        heap_desc.Type = type;
        heap_desc.NodeMask = gpu_mask;
        heap_desc.Flags = type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER ?
            D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        auto r = dev->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&heap));
        dx12u::throw_if_error(r);

        slot_size = dev->GetDescriptorHandleIncrementSize(type);
    }

    descriptor_heap::descriptor_heap(descriptor_heap&& other) noexcept
        : heap(other.heap), num_slot(other.num_slot), slot_size(other.slot_size)
    {
        other.heap = nullptr;
        other.num_slot = 0;
        other.slot_size = 0;
    }

    descriptor_heap& descriptor_heap::operator = (descriptor_heap&& other) noexcept
    {
        if (other.heap.Get() != heap.Get())
        {
            heap = other.heap;
            num_slot = other.num_slot;
            slot_size = other.slot_size;
            other.heap = nullptr;
            other.num_slot = 0;
            other.slot_size = 0;
        }

        return *this;
    }

} // namespace dx12u

