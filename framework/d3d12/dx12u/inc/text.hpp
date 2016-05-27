
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
* @file      text.hpp
* @brief     dx12 text utilities
*/

#ifndef DX12U_TEXT_HPP
#define DX12U_TEXT_HPP

#include <cmd_mgr.hpp>
#include <descriptor_heap.hpp>
#include <pso.hpp>
#include <texture.hpp>
#include <string>
#include <vector>

namespace dx12u
{
    /**
    * @brief 2d text container
    */
    class text_2d
    {
        std::size_t max_word_sz = 0;
        std::size_t max_num_word = 0;

        // srd heap
        // slot 0: text texture srv
        // slot 1 -> max_frame * max_num_word: cbv list
        dx12u::descriptor_heap srd_heap{};

        // texture with character bitmaps
        resource font_texture{ nullptr };

        // pso
        dx12u::root_signature rs{ nullptr };
        dx12u::pipeline_state_object  pso{};

        // constant buffer 
        dx12u::resource  cb_mem{ nullptr };
        std::uint8_t*    cb_ptr = nullptr;

        struct word
        {
            float x;
            float y;
            float char_w;
            float char_h;
            std::string str;
        };

        using world_list = std::vector<word>;
        using frame_word_list = std::vector<world_list>;

        frame_word_list words;

    public:

        // disable copy and assignment
        text_2d(text_2d const&) = delete;
        text_2d& operator = (text_2d const&) = delete;

        /**
        * @brief create an invalid text container
        */
        text_2d() = default;

        /**
        * @brief move a text_2d container
        */
        text_2d(text_2d&&);

        /**
        * @brief move a text_2d container
        */
        text_2d& operator = (text_2d&&);

        /**
        * @brief create a text container
        * @param dev dx12 device
        * @param max_str_size maximum number of characters in one string
        * @param max_num_str maximum number of strings in one frame
        * @param max_buffered_frame maximum number of buffered frames
        * @param enable_filtering if true the text will be filtered
        */
        text_2d(device const& dev, std::size_t max_str_size, std::size_t max_num_str, std::size_t max_buffered_frame, bool enable_filtering);

        /**
        * @brief add text
        * @param frame_id the frame for which the text is added. frame_id cannot exceed max_buffered_frame
        * @param x text start x position in clip space
        * @param y text start y position in clip space
        * @param char_w one character width in clip space
        * @param char_h one character height in clip space
        * @param str text to add
        * @note the function fails if frame_id >= max_buffered_frame or max_num_str is reached or str.size() >= max_str_size
        */
        void add_text(std::size_t frame_id, float x, float y, float char_w, float char_h, std::string const& str);

        /**
        * @brief clear text. Typically called when a frame begins
        * @param frame_id the frame for which the text is cleared
        */
        void clear(std::size_t frame_id);

        /**
        * @brief draw the text. Typically called at the end of a frame
        * @param frame_id the frame for which the text is drawn
        * @param cl d3d12 command list
        * @note the function changes the bound PSO and descriptor heap. The topology must be D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST and no vertex buffer is bound
        */
        void draw(std::size_t frame_id, ID3D12GraphicsCommandList* cl);

        /**
        * @brief check the validity of the container
        * @param frame_id the frame for which the text is drawn
        * @return true if valid. false otherwise
        */
        bool is_valid() const noexcept
        {
            return !words.empty();
        }

        /**
        * @brief get optimal minimum text char width given a window resolutin
        * @param window_width window width in pixel
        * @return char width in clip space
        */
        float get_char_w(std::size_t window_width);

        /**
        * @brief get optimal minimum text char width given a window resolutin
        * @param window_height window height in pixel
        * @return char height in clip space
        */
        float get_char_h(std::size_t window_height);
    };

} // namespace dx12u



#endif // DX12U_TEXT_HPP

