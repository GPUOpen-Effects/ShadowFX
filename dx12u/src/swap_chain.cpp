
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
* @file      swap_chain.hpp
* @brief     dx12 swap chain implementation
*/

#include <swap_chain.hpp>
#include <algorithm>

using std::uint32_t;
using std::size_t;

namespace dx12u
{
    swap_chain::~swap_chain()
    {
    }

    swap_chain::swap_chain(cmd_queue const& q, HWND hwnd, std::size_t w, std::size_t h,
        std::size_t buffer_count,
        DXGI_FORMAT format,
        std::size_t num_sample,
        std::size_t quality,
        DXGI_SWAP_EFFECT mode
        )
    {
        if (!q.is_valid())
        {
            throw error{ "swap chain creation failed. invalid command queue" };
        }

        if (!w || !h || !num_sample)
        {
            throw error{ "swap chain buffer size of 0" };
        }

        if (!buffer_count)
        {
            throw error{ "swap chain buffer count of 0" };
        }

        desc = DXGI_SWAP_CHAIN_DESC{};
        desc.BufferCount = static_cast<uint32_t>(buffer_count);
        desc.BufferDesc.Width = static_cast<uint32_t>(w);
        desc.BufferDesc.Height = static_cast<uint32_t>(h);
        desc.BufferDesc.Format = format;
        desc.BufferDesc.RefreshRate.Numerator = 0;
        desc.BufferDesc.RefreshRate.Denominator = 1;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.SwapEffect = mode;
        desc.OutputWindow = hwnd;
        desc.SampleDesc.Count = static_cast<uint32_t>(num_sample);
        desc.SampleDesc.Quality = static_cast<uint32_t>(quality);
        desc.Windowed = true;
        desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        com_ptr<IDXGISwapChain> swap_chain_v1;
        auto r = get_factory()->CreateSwapChain(q.get_com_ptr().Get(), &desc, &swap_chain_v1);
        throw_if_error(r, "swap chain creation failed. ");

        // set swap_chain_v1 to the new v3 swap chain
        r = swap_chain_v1.As(&chain);
        throw_if_error(r, "swap chain creation failed. ");

        // disable automatic swap chain changes
        get_factory()->MakeWindowAssociation(hwnd, DXGI_MWA_NO_WINDOW_CHANGES);
    }

    swap_chain::swap_chain(swap_chain&& other) noexcept
        : chain(other.chain), desc(other.desc), prev_fullscreen_state(other.prev_fullscreen_state)
    {
        other.chain = nullptr;
    }

    swap_chain& swap_chain::operator = (swap_chain&& other) noexcept
    {
        if (other.chain.Get() != chain.Get())
        {
            if (is_valid())
            {
                set_fullscreen(false);
            }
            chain = other.chain;
            desc = other.desc;
            prev_fullscreen_state = other.prev_fullscreen_state;
            other.chain = nullptr;
        }

        return *this;
    }

    resource swap_chain::get_buffer(std::size_t idx) const
    {
        if (idx >= get_num_buffer())
        {
            throw error{"out of range swap chain buffer access"};
        }

        resource rsrc{ nullptr };
        auto r = chain->GetBuffer(static_cast<uint32_t>(idx), IID_PPV_ARGS(&rsrc));
        throw_if_error(r, "cannot get swap chain buffer. ");

        return rsrc;
    }

    void swap_chain::set_fullscreen(bool state)
    {
        assert(is_valid());
        if (prev_fullscreen_state == state)
        {
            return;
        }
        prev_fullscreen_state = state;
        mode_switch = true;
        auto r = chain->SetFullscreenState(state, nullptr);
        throw_if_error(r, "failed to set fullscreen state");
        mode_switch = false;
        r = chain->ResizeBuffers(desc.BufferCount, desc.BufferDesc.Width, desc.BufferDesc.Height, desc.BufferDesc.Format, desc.Flags);
        throw_if_error(r, "failed to resize swap chain");
    }

    bool swap_chain::is_fullscreen()
    {
        assert(is_valid());
        BOOL fullscreen_mode = false;
        chain->GetFullscreenState(&fullscreen_mode, nullptr);
        return !!fullscreen_mode;
    }

    void swap_chain::present() const
    {
        assert(is_valid());
        auto r = chain->Present(0, 0);
        throw_if_error(r, "present failed.");
    }

    swap_chain::resolution_list swap_chain::query_modes()
    {
        assert(is_valid());

        com_ptr<IDXGIOutput> out{};
        chain->GetContainingOutput(&out);

        uint32_t num_mode = 0;
        auto r = out->GetDisplayModeList(desc.BufferDesc.Format, 0, &num_mode, nullptr);
        throw_if_error(r, "swap chain query modes failed.");

        std::vector<DXGI_MODE_DESC> mode_desc_list(num_mode);
        r = out->GetDisplayModeList(desc.BufferDesc.Format, 0, &num_mode, mode_desc_list.data());
        throw_if_error(r, "swap chain query modes failed.");

        resolution_list mode_list(num_mode);
        for (size_t i = 0; i < mode_list.size(); ++i)
        {
            mode_list[i].x = mode_desc_list[i].Width;
            mode_list[i].y = mode_desc_list[i].Height;
        }

        mode_list.erase(std::unique(std::begin(mode_list), std::end(mode_list)), std::end(mode_list));
        return mode_list;
    }

    tml::uvec2 swap_chain::query_closest_mode(std::size_t w, std::size_t h)
    {
        assert(is_valid());

        com_ptr<IDXGIOutput> out{};
        chain->GetContainingOutput(&out);

        DXGI_MODE_DESC mode_desc_in{ static_cast<uint32_t>(w), static_cast<uint32_t>(h) };
        DXGI_MODE_DESC mode_desc_out{ 0 };
        mode_desc_in.Format = desc.BufferDesc.Format;
        auto r = out->FindClosestMatchingMode(&mode_desc_in, &mode_desc_out, nullptr);
        throw_if_error(r, "swap chain query closest mode failed.");

        return tml::uvec2{ mode_desc_out.Width, mode_desc_out.Height };
    }

} // namespace dx12u

