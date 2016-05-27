
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
* @brief     dx12 descriptor heap utilities
*/

#ifndef DX12UTIL_DESCRIPTOR_HEAP_HPP
#define DX12UTIL_DESCRIPTOR_HEAP_HPP

#include <cmd_mgr.hpp>
#include <cassert>
#include <array>
#include <type_traits>

namespace dx12u
{
    /**
    * @brief descriptor heap manager
    */
    class descriptor_heap
    {
        using desc_heap_com_ptr = com_ptr<ID3D12DescriptorHeap>;

        desc_heap_com_ptr heap{ nullptr };
        std::size_t num_slot = 0; // number of slots in the heap
        std::size_t slot_size = 0; // slot size

    public:
        /**
        * @brief create an invalid descriptor heap
        */
        descriptor_heap()
        {}

        /**
        * @brief create a descriptor heap
        * @param dev device
        * @param type descriptor heap type
        * @param n number of heap slots
        * @param gpu_ordinal gpu ordinal. a negative value can be used if there's only one gpu in the chain
        */
        explicit descriptor_heap(ID3D12Device* dev, D3D12_DESCRIPTOR_HEAP_TYPE type, std::size_t n, int gpu_ordinal = -1);

        /**
        * @brief non copyable
        */
        descriptor_heap(descriptor_heap const&) = delete;

        /**
        * @brief non copy-assignable
        */
        descriptor_heap& operator = (descriptor_heap const&) = delete;

        /**
        * @brief move a heap. the moved from object becomes invalid
        * @param other heap to move from
        */
        descriptor_heap(descriptor_heap&& other) noexcept;

        /**
        * @brief move assign a heap. the moved from object becomes invalid
        * @param other heap to move from
        */
        descriptor_heap& operator = (descriptor_heap&& other) noexcept;

        /**
        * @brief access the underlying heap com ptr
        * @return heap com ptr
        * @note the ptr is null if the heap is invalid
        */
        desc_heap_com_ptr const& get_com_ptr() const noexcept
        {
            return heap;
        }

        /**
        * @brief check whether the pso is valid of not
        * @return true if the pso is valid; false otherwise
        */
        bool is_valid() const noexcept
        {
            return heap.Get() != nullptr;
        }

        /**
        * @brief get cpu descriptor handle at slot i
        * @param i slot index
        * @return descriptor handle
        * @note the behavior is undefined if the heap is invalid
        */
        D3D12_CPU_DESCRIPTOR_HANDLE get_cpu_handle(std::size_t i) const
        {
            assert(is_valid() && slot_size > 0 && i < num_slot);
            return { heap->GetCPUDescriptorHandleForHeapStart().ptr + i * slot_size };
        }

        /**
        * @brief get gpu descriptor handle at slot i
        * @param i slot index
        * @return descriptor handle
        * @note the behavior is undefined if the heap is invalid
        */
        D3D12_GPU_DESCRIPTOR_HANDLE get_gpu_handle(std::size_t i) const
        {
            assert(is_valid() && slot_size > 0 && i < num_slot);
            return{ heap->GetGPUDescriptorHandleForHeapStart().ptr + static_cast<std::uint64_t>(i * slot_size) };
        }
    };

    /**
    * @brief bind one descriptor heap
    * @param cl a command list
    * @param heap descriptor heap
    */
    inline void bind_descriptor_heap(ID3D12GraphicsCommandList* cl, ID3D12DescriptorHeap* heap)
    {
        cl->SetDescriptorHeaps(1, &heap);
    }

} // namespace dx12u



#endif // DX12UTIL_DESCRIPTOR_HEAP_HPP
