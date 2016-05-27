
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
* @brief     dx12 swap chain
*/

#ifndef DX12UTIL_SWAPCHAIN_HPP
#define DX12UTIL_SWAPCHAIN_HPP

#include <cmd_mgr.hpp>
#include <memory.hpp>
#include <cstddef>
#include <tml/vec.hpp>

namespace dx12u
{
    /**
    * @brief a swap chain wrapper
    */
    class swap_chain
    {
        
    public:
        using swap_chain_com_ptr = com_ptr<IDXGISwapChain3>; //!< swap_chain com ptr

    private:
        swap_chain_com_ptr chain{ nullptr };
        DXGI_SWAP_CHAIN_DESC desc{};
        bool prev_fullscreen_state = false;
        bool mode_switch = false; // true while a mode switch is being executed

    public:

        /**
        * @brief create an invalid swap chain
        */
        swap_chain()
        {}

        /**
        * @brief release the swap chain
        */
        ~swap_chain();

        /**
        * @brief create a swap chain
        * @param q command queue
        * @param hwnd window handle
        * @param w width
        * @param h height
        * @param buffer_count number of buffers in the chain
        * @param format color buffer format
        * @param num_sample multi sample count
        * @param quality multi sample quality
        * @param mode swap mode
        */
        explicit swap_chain(cmd_queue const& q, HWND hwnd, std::size_t w, std::size_t h,
            std::size_t buffer_count = 2,
            DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM,
            std::size_t num_sample = 1,
            std::size_t quality = 0,
            DXGI_SWAP_EFFECT mode = DXGI_SWAP_EFFECT_FLIP_DISCARD
            );

        /**
        * @brief non copyable
        */
        swap_chain(swap_chain const&) = delete;

        /**
        * @brief non copy-assignable
        */
        swap_chain& operator = (swap_chain const&) = delete;

        /**
        * @brief move a swap chain. the moved from object becomes invalid
        * @param other swap chain to move from
        */
        swap_chain(swap_chain&& other) noexcept;

        /**
        * @brief move assign a swap chain. the moved from object becomes invalid
        * @param other swap chain to move from
        */
        swap_chain& operator = (swap_chain&& other) noexcept;

        /**
        * @brief get the buffer resource
        * @param idx index in the chain
        * @return resource
        */
        resource get_buffer(std::size_t idx) const;

        /**
        * @brief get the buffer index of the current back buffer
        * @return current back buffer buffer index
        * @note the behavior is undefined if the swap chain is invalid
        */
        std::size_t get_current_buffer_idx() const
        {
            assert(is_valid());
            return chain->GetCurrentBackBufferIndex();
        }

        /**
        * @brief get the number of buffers in the chain
        * @return number of buffers in the chain
        */
        std::size_t get_num_buffer() const noexcept
        {
            return desc.BufferCount;
        }

        /**
        * @brief access the underlying swap chain com ptr
        * @return swap chain com ptr
        * @note the ptr is null if the swap chain is invalid
        */
        swap_chain_com_ptr const& get_com_ptr() const noexcept
        {
            return chain;
        }

        /**
        * @brief check whether the swap chain is valid of not
        * @return true if the swap chain is valid; false otherwise
        */
        bool is_valid() const noexcept
        {
            return chain.Get() != nullptr;
        }

        /**
        * @brief enable/disable fullscreen
        * @param state true to set fullscreen; false to return to windowed mode
        * @note undefined behavior if the swap chain is invalid
        */
        void set_fullscreen(bool state);

        /**
        * @brief check if the swap chain is in fullscreen mode
        * @return true if the swap chain is in fullscreen mode; false otherwise
        */
        bool is_fullscreen();

        /**
        * @brief present frame
        */
        void present() const;

        /**
        * @brief the swap chain is busy
        * @return true if the swap chain is busy; false otherwise
        * @note the swap chain is busy when a mode switch is happening
        */
        bool is_busy() const noexcept
        {
            return mode_switch;
        }

        /**
        * @brief get the swap chain size
        * @return swap chain size
        * @note the swap chain is busy when a mode switch is happening
        */
        tml::uvec2 get_size() const noexcept
        {
            return tml::uvec2{ desc.BufferDesc.Width, desc.BufferDesc.Height };
        }

        /**
        * @brief a list of width/height pairs
        */
        using resolution_list = std::vector<tml::uvec2>;

        /**
        * @brief query supported resolution modes
        * @return list of supported resolution
        * @note undefined behavior if the swap chain is invalid
        */
        resolution_list query_modes();

        /**
        * @brief get the closest matching resolution mode
        * @param w width
        * @param h height
        * @return supported resolution closest to w x h
        * @note undefined behavior if the swap chain is invalid
        */
        tml::uvec2 query_closest_mode(std::size_t w, std::size_t h);
    };

} // namespace dx12u


#endif // DX12UTIL_SWAPCHAIN_HPP
