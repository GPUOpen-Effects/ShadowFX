
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
* @file      cmd_mgr.hpp
* @brief     dx12 command manager
*/

#ifndef DX12UTIL_CMD_MGR_HPP
#define DX12UTIL_CMD_MGR_HPP

#include <device.hpp>
#include <vector>
#include <atomic>
#include <cassert>
#include <algorithm>
#include <mutex>

namespace dx12u
{
    /**
    * @brief a command list
    */
    using gfx_cmd_list = com_ptr<ID3D12GraphicsCommandList>;

    /**
    * @brief a list of command lists
    */
    using gfx_cmd_list_list = std::vector<gfx_cmd_list>;

    /**
    * @brief a command allocator
    */
    class cmd_allocator
    {
    public:
        using cmd_allocator_com_ptr = com_ptr<ID3D12CommandAllocator>; //!< cmd_allocator com ptr
        using cmd_allocator_ptr = ID3D12CommandAllocator*; //!< cmd_allocator ptr
        using cmd_allocator_const_ptr = ID3D12CommandAllocator const*; //!< cmd_allocator const ptr

    private:
        cmd_allocator_com_ptr allocator{ nullptr };
        device                dev{ nullptr };
        std::mutex            mux; // for the safe_reset
    public:
        /**
        * @brief create an invalid command allocator
        */
        cmd_allocator()
        {}

        /**
        * @brief create a command allocator
        * @param engine command list engine type
        * @param dvce dx12 device
        */
        explicit cmd_allocator(device const& dvce, D3D12_COMMAND_LIST_TYPE engine = D3D12_COMMAND_LIST_TYPE_DIRECT);

        /**
        * @brief non copyable
        */
        cmd_allocator(cmd_allocator const&) = delete;

        /**
        * @brief non copy-assignable
        */
        cmd_allocator& operator = (cmd_allocator const&) = delete;

        /**
        * @brief move a command allocator. the moved from object becomes invalid
        * @param other command allocator to move from
        */
        cmd_allocator(cmd_allocator&& other) noexcept;

        /**
        * @brief move assign a command allocator. the moved from object becomes invalid
        * @param other command allocator to move from
        */
        cmd_allocator& operator = (cmd_allocator&& other) noexcept;

        /**
        * @brief access the underlying command allocator com ptr
        * @return command allocator com ptr
        * @note the ptr is null if the command allocator is invalid
        */
        cmd_allocator_com_ptr const& get_com_ptr() const noexcept
        {
            return allocator;
        }

        /**
        * @brief access the underlying command allocator ptr
        * @return command allocator ptr
        * @note the ptr is null if the command allocator is invalid
        */
        cmd_allocator_const_ptr get_ptr() const noexcept
        {
            return allocator.Get();
        }

        /**
        * @brief access the underlying command allocator ptr
        * @return command allocator ptr
        * @note the ptr is null if the command allocator is invalid
        */
        cmd_allocator_ptr get_ptr() noexcept
        {
            return allocator.Get();
        }

        /**
        * @brief check whether the allocator is valid of not
        * @return true if the allocator is valid; false otherwise
        */
        bool is_valid() const noexcept
        {
            return allocator.Get() != nullptr;
        }

        /**
        * @brief create a new gfx command list
        * @param engine command list engine type
        * @param gpu_ordinal gpu ordinal. a negative value can be used if there's only one gpu in the chain
        * @return command list
        */
        gfx_cmd_list alloc(D3D12_COMMAND_LIST_TYPE engine = D3D12_COMMAND_LIST_TYPE_DIRECT, int gpu_ordinal = -1) const;

        /**
        * @brief reset
        * @param free the allocated memory
        * @return true if the reset succeeds; false otherwise
        * @note use safe_reset if possible as it's thread safe and checks for errors
        * @note resetting a command list allows reusing it (can be done right after submitting) but won't free memory (as it may be in use by the gpu)
        * @note it's this function that frees the memory (on AMD a pool allocator is used and this function moves all blocks to the free list)
        * @note a command list reset allows reusing potential cpu memory but the actual gpu command buffer won't be freed until calling this function
        */
        bool reset() const
        {
            assert(is_valid());
            auto r = allocator->Reset();
            assert(r == S_OK);
            (void*)(&r); // unreferenced
            return allocator->Reset() == S_OK;
        }

        /**
        * @brief thread safe version of reset with error checking
        * @param free the allocated memory
        */
        void safe_reset();
    };

    /**
    * @brief a command queue wrapper
    */
    class cmd_queue
    {
    public:
        using cmd_queue_com_ptr = com_ptr<ID3D12CommandQueue>; //!< cmd_queue com ptr

    private:
        cmd_queue_com_ptr queue{ nullptr };
        // fence for quick sync
        HANDLE fence_event = CreateEvent(nullptr, false, false, nullptr);
        com_ptr<ID3D12Fence> fence{ nullptr };
        std::atomic_ullong fence_value{ 1 };
        // user fence
        com_ptr<ID3D12Fence> user_fence{ nullptr };

    public:
        /**
        * @brief create an invalid command queue
        */
        cmd_queue()
        {}

        /**
        * @brief create a command queue
        * @param dvce dx12 device
        * @param engine command list engine type
        * @param gpu_ordinal gpu ordinal. a negative value can be used if there's only one gpu in the chain
        */
        explicit cmd_queue(device const& dvce, D3D12_COMMAND_LIST_TYPE engine = D3D12_COMMAND_LIST_TYPE_DIRECT, int gpu_ordinal = -1);

        /**
        * @brief non copyable
        */
        cmd_queue(cmd_queue const&) = delete;

        /**
        * @brief non copy-assignable
        */
        cmd_queue& operator = (cmd_queue const&) = delete;

        /**
        * @brief move a command queue. the moved from object becomes invalid
        * @param other command queue to move from
        */
        cmd_queue(cmd_queue&& other) noexcept;

        /**
        * @brief move assign a command queue. the moved from object becomes invalid
        * @param other command queue to move from
        */
        cmd_queue& operator = (cmd_queue&& other) noexcept;

        /**
        * @brief access the underlying command queue com ptr
        * @return command queue com ptr
        * @note the ptr is null if the command queue is invalid
        */
        cmd_queue_com_ptr const& get_com_ptr() const noexcept
        {
            return queue;
        }

        /**
        * @brief check whether the queue is valid of not
        * @return true if the queue is valid; false otherwise
        */
        bool is_valid() const noexcept
        {
            return queue.Get() != nullptr;
        }

        /**
        * @brief wait until the queue is empty
        */
        void sync();

        /**
        * @brief set a user fence value
        * @param value new fence value
        */
        void set_fence(std::uint64_t value);

        /**
        * @brief check if the target user fence value is reached
        * @param target_value target fence value 
        * @return true if the target fence value is reached; false otherwise
        * @note the behavior is undefined if the command queue is invalid
        */
        bool check_fence(std::uint64_t target_value) const;

        /**
        * @brief wait for the target user fence value
        * @param target_value target fence value
        */
        void wait(std::uint64_t target_value) const;

        /**
        * @brief push a command list in the queue for execution
        * @param cl command list
        * @note the behavior is undefined if the command queue is invalid
        */
        void push(gfx_cmd_list const& cl) const
        {
            assert(is_valid() && cl.Get() != nullptr);

            ID3D12CommandList* tmp = cl.Get();
            queue->ExecuteCommandLists(1, &tmp);
        }

        /**
        * @brief push a list of command lists in the queue for execution
        * @param cl_list command list
        * @note the behavior is undefined if the command queue is invalid
        */
        void push(gfx_cmd_list_list const& cl_list) const
        {
            assert(is_valid());

            std::vector<ID3D12CommandList*> tmp{ cl_list.size() };
            std::transform(std::begin(cl_list), std::end(cl_list), std::begin(tmp), [](auto x) {return x.Get(); });
            queue->ExecuteCommandLists(static_cast<uint32_t>(tmp.size()), tmp.data());
        }
    };


    // quick functions to update cmd lists

    /**
    * @brief set one scissor rect
    * @param cl_list command list
    * @param scissor x top left x
    * @param scissor y top left y
    * @param scissor width
    * @param scissor height
    */
    inline void set_scissor(ID3D12GraphicsCommandList* cl, std::size_t x, std::size_t y, std::size_t w, std::size_t h)
    {
        assert(cl != nullptr);
        D3D12_RECT scissor
        {
            static_cast<int>(x),
            static_cast<int>(y),
            static_cast<int>(x) + static_cast<int>(w),
            static_cast<int>(y) + static_cast<int>(h)
        };
        cl->RSSetScissorRects(1, &scissor);
    }

    /**
    * @brief set one viewport
    * @param cl_list command list
    * @param x viewport top left x coodrinate
    * @param y viewport top left y coodrinate
    * @param w viewport width
    * @param h viewport height
    */
    inline void set_viewport(ID3D12GraphicsCommandList* cl, float x, float y, float w, float h)
    {
        assert(cl != nullptr);
        D3D12_VIEWPORT viewport
        {
            x, y,
            w, h,
            0.f, 1.f
        };
        cl->RSSetViewports(1, &viewport);
    }

    /**
    * @brief set one viewport
    * @param cl_list command list
    * @param x viewport top left x coodrinate
    * @param y viewport top left y coodrinate
    * @param w viewport width
    * @param h viewport height
    */
    inline void set_viewport(ID3D12GraphicsCommandList* cl, std::size_t w, std::size_t h)
    {
        set_viewport(cl, 0.f, 0.f, static_cast<float>(w), static_cast<float>(h));
    }

    /**
    * @brief set one viewport and scissor
    * @param cl_list command list
    * @param w viewport and scissor width
    * @param h viewport and scissor height
    */
    inline void set_viewport_scissor(ID3D12GraphicsCommandList* cl, std::size_t w, std::size_t h)
    {
        set_viewport(cl, w, h);
        set_scissor(cl, 0, 0, w, h);
    }

} // namespace dx12u


#endif // DX12UTIL_CMD_MGR_HPP
 