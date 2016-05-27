
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
* @file      cmd_mgr.cpp
* @brief     dx12 command manager implementation
*/

#include <cmd_mgr.hpp>

using std::uint8_t;
using std::uint32_t;
using std::uint64_t;
using std::size_t;

namespace dx12u
{

    cmd_queue::cmd_queue(device const& dvce, D3D12_COMMAND_LIST_TYPE engine, int gpu_ordinal)
    {
        if (!dvce.Get())
        {
            throw error{ "command queue creation failed. null device" };
        }

        std::uint32_t gpu_mask = gpu_ordinal < 0 ? 0 : (1u << static_cast<std::uint32_t>(gpu_ordinal));

        D3D12_COMMAND_QUEUE_DESC queue_desc = {};
        queue_desc.Type = engine;
        queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queue_desc.NodeMask = gpu_mask;

        auto r = dvce->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&queue));
        throw_if_error(r, "command queue creation failed. ");

        r = dvce->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
        throw_if_error(r, "fence creation failed. ");

        r = dvce->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&user_fence));
        throw_if_error(r, "fence creation failed. ");
    }

    cmd_queue::cmd_queue(cmd_queue&& other) noexcept
        : queue(other.queue), 
        fence(other.fence), fence_value(other.fence_value.load()), user_fence(other.user_fence)
    {
        other.queue = nullptr;
        other.fence = nullptr;
        other.user_fence = nullptr;
        other.fence_value = 0;
        other.fence_event = nullptr;
    }

    cmd_queue& cmd_queue::operator = (cmd_queue&& other) noexcept
    {
        if (other.queue.Get() != queue.Get())
        {
            queue = other.queue;
            fence = other.fence;
            user_fence = other.user_fence;
            fence_value = other.fence_value.load();
            other.queue = nullptr;
            other.fence = nullptr;
            other.user_fence = nullptr;
            other.fence_value = 0;
            other.fence_event = nullptr;
        }

        return *this;
    }

    void cmd_queue::sync()
    {
        assert(!!fence.Get() && !!fence_event);

        if (!is_valid())
        {
            throw error{ "failed to sync command queue. invalid command queue" };
        }

        // set gpu fence value
        uint64_t fvalue = fence_value.load();
        auto r = queue->Signal(fence.Get(), fvalue);
        throw_if_error(r, "failed to signal gpu fence. ");

        // wait for fence
        if (fence->GetCompletedValue() < fvalue)
        {
            r = fence->SetEventOnCompletion(fvalue, fence_event);
            throw_if_error(r, "failed to set fence event. ");

            if (WaitForSingleObject(fence_event, INFINITE) == WAIT_FAILED)
            {
                throw_if_win32_error("failed to wait for command queue sync event. ");
            }
        }

        // inrement fence value
        ++fence_value;
    }

    void cmd_queue::set_fence(std::uint64_t value)
    {
        if (!is_valid())
        {
            throw error{ "failed to set fence. invalid command queue" };
        }

        auto r = queue->Signal(user_fence.Get(), value);
        throw_if_error(r, "failed to signal gpu fence. ");
    }

    bool cmd_queue::check_fence(std::uint64_t target_value) const
    {
        assert(is_valid());
        return user_fence->GetCompletedValue() >= target_value;
    }

    void cmd_queue::wait(std::uint64_t target_value) const
    {
        if (check_fence(target_value))
        {
            return;
        }

        auto r = user_fence->SetEventOnCompletion(target_value, fence_event);
        throw_if_error(r, "failed to set fence event. ");

        if (WaitForSingleObject(fence_event, INFINITE) == WAIT_FAILED)
        {
            throw_if_win32_error("failed to wait for command queue sync event. ");
        }
    }

    cmd_allocator::cmd_allocator(device const & dvce, D3D12_COMMAND_LIST_TYPE engine)
        : dev(dvce)
    {
        if (!dvce.Get())
        {
            throw error{ "command allocator creation failed. null device" };
        }

        auto r = dvce->CreateCommandAllocator(engine, IID_PPV_ARGS(&allocator));
        dx12u::throw_if_error(r);
    }

    cmd_allocator::cmd_allocator(cmd_allocator && other) noexcept
        : allocator(other.allocator)
    {
        other.allocator = nullptr;
    }

    cmd_allocator& cmd_allocator::operator = (cmd_allocator&& other) noexcept
    {
        if (other.allocator.Get() != allocator.Get())
        {
            allocator = other.allocator;
            dev = other.dev;
            other.allocator = nullptr;
            other.dev = nullptr;
        }

        return *this;
    }

    gfx_cmd_list cmd_allocator::alloc(D3D12_COMMAND_LIST_TYPE engine, int gpu_ordinal) const
    {
        if (!is_valid())
        {
            throw error{ "command list creation failed. invalid command allocator" };
        }

        std::uint32_t gpu_mask = gpu_ordinal < 0 ? 0 : (1u << static_cast<std::uint32_t>(gpu_ordinal));
        gfx_cmd_list cl{ nullptr };
        auto r = dev->CreateCommandList(gpu_mask, engine, allocator.Get(), nullptr, IID_PPV_ARGS(&cl));
        dx12u::throw_if_error(r);
        return cl;
    }

    void cmd_allocator::safe_reset()
    {
        std::unique_lock<std::mutex> lock{ mux };

        if (!is_valid())
        {
            throw error{ "command list reset failed. invalid command allocator" };
        }

        auto r = allocator->Reset();
        dx12u::throw_if_error(r);
    }

} // namespace dx12u

