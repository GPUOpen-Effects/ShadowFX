
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
* @file      text.cpp
* @brief     dx12 text implementation
*/


#include <text.hpp>
#include <texture.hpp>
#include <tml/vec.hpp>
#include <cassert>
#include <gu/gu.hpp>
#include <fstream>

using std::size_t;
using std::uint8_t;
using std::uint32_t;

namespace
{
    struct cbuffer_data
    {
        tml::vec4 scale_bias; // xy bias, zw scale. zw multiplied by 2 in CPU
        float*    u_start; // uv start for each char in the word
    };

    size_t get_u_start_offset()
    {
        return sizeof(tml::vec4); // u_start comes after scale_bias
    }

    size_t get_cb_size(size_t max_str_size)
    {
        assert(max_str_size > 0);
        size_t num_u_start = (max_str_size - 1) / 4 + 1;

        size_t sz = num_u_start * sizeof(tml::vec4) + sizeof(tml::vec4); // uv_start + scale_bias

        return gu::align(sz, dx12u::get_const_buffer_alignment());
    }


    namespace global
    {
        size_t const font_texture_width = 950;
        size_t const font_texture_height = 20;
        size_t const font_texture_char_w = 10;
        size_t const font_texture_bias = 32;
    }

    std::vector<uint8_t> load_font_texture()
    {
        std::ifstream ifs("../media/dxut_text/text.raw", std::ios::binary);
        return std::vector<uint8_t>(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
    }
}

namespace dx12u
{

    text_2d::text_2d(device const& dev, std::size_t max_str_size, std::size_t max_num_str, std::size_t max_buffered_frame, bool enable_filtering)
        : max_word_sz(max_str_size), max_num_word(max_num_str)
    {
        assert(max_str_size > 0 && max_num_str > 0 && max_buffered_frame > 0);

        if (dev.Get() == nullptr)
        {
            throw error{ "null device" };
        }

        words.resize(max_buffered_frame);

        size_t const num_cb = max_num_str * max_buffered_frame;
        size_t const num_tex = 1; // 1 text texture

        srd_heap = dx12u::descriptor_heap{ dev.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, num_cb + num_tex };

        auto cb_sz = get_cb_size(max_str_size);
        auto cb_total_sz = get_cb_size(max_str_size) * num_cb;

        // alloc and upload texture
        {
            auto font_data = load_font_texture();

            gu::mip_lvl_texture_view mip_view{};
            mip_view.width = global::font_texture_width;
            mip_view.height = global::font_texture_height;
            mip_view.pitch = mip_view.width;
            mip_view.slice_sz = font_data.size();
            mip_view.data = font_data.data();

            gu::texture_view tex_view{};
            tex_view.bpp = 1;
            tex_view.num_lvl = 1;
            tex_view.mip = &mip_view;

            dx12u::cmd_queue q{ dev };
            dx12u::cmd_allocator allocator{ dev };
            dx12u::gfx_cmd_list  cl = allocator.alloc();

            auto tex = dx12u::make_texture(dev, cl, tex_view, false, false, DXGI_FORMAT_R8_UNORM);
            cl->Close();
            q.push(cl);
            q.sync();

            font_texture = tex.texture_rsrc;
            dev->CreateShaderResourceView(font_texture.Get(), &tex.srv_desc, srd_heap.get_cpu_handle(0));
        }

        // alloc cb_mem
        {
            auto upload_heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            auto buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(cb_total_sz);
            auto r = dev->CreateCommittedResource(&upload_heap, D3D12_HEAP_FLAG_NONE,
                &buffer_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&cb_mem));
            dx12u::throw_if_error(r);

            for (size_t i = 0; i < num_cb; ++i)
            {
                // sh_mask_cb views
                D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc = {};
                cbv_desc.SizeInBytes = static_cast<uint32_t>(cb_sz);
                cbv_desc.BufferLocation = cb_mem->GetGPUVirtualAddress() + i * cb_sz;
                dev->CreateConstantBufferView(&cbv_desc, srd_heap.get_cpu_handle(num_tex + i));
            }

            r = cb_mem->Map(0, nullptr, reinterpret_cast<void**>(&cb_ptr));
            dx12u::throw_if_error(r);
            assert(!!cb_ptr);
            memset(cb_ptr, 0, cb_total_sz);
        }

        // pso
        {
            // root signature
            auto filter = enable_filtering ?  D3D12_FILTER_MIN_MAG_MIP_LINEAR : D3D12_FILTER_MIN_MAG_MIP_POINT;
            dx12u::descriptor_sig_list descriptor_table0{};
            dx12u::descriptor_sig_list descriptor_table1{};
            descriptor_table0.append(dx12u::descriptor_sig{ dx12u::descriptor_type::srv, 0, 0, dx12u::shader_mask::ps });
            descriptor_table1.append(dx12u::descriptor_sig{ dx12u::descriptor_type::cbv, 0, 0, dx12u::shader_mask::vs });
            dx12u::descriptor_sig_list dsl{};
            dsl.append(descriptor_table0); // TODO move to root once the number of sgpr is definite
            dsl.append(descriptor_table1);
            dx12u::static_sampler_list sl =
            {
                dx12u::make_default_static_sampler(0, filter),
            };
            rs = dx12u::make_root_signature(dev.Get(), dsl, sl);

            // shader preprocessor definitions
            auto const texture_char_w = static_cast<double>(global::font_texture_char_w) / static_cast<double>(global::font_texture_width);
            using macro = std::pair<std::string, std::string>;
            macro max_str_macro = macro{ "MAX_STR_SZ", std::to_string(max_str_size) };
            macro char_w_macro = macro{ "CHAR_WIDTH", std::to_string(texture_char_w) };
            D3D_SHADER_MACRO def[] =
            {
                max_str_macro.first.c_str(), max_str_macro.second.c_str(),
                char_w_macro.first.c_str(), char_w_macro.second.c_str(),
                nullptr, nullptr
            };
            // shader blobs
            auto vs = dx12u::compile_from_file("../../framework/d3d12/shader/ui/text2d.hlsl", def, "vs_main", "vs_5_0");
            auto ps = dx12u::compile_from_file("../../framework/d3d12/shader/ui/text2d.hlsl", def, "ps_main", "ps_5_0");
            // pso
            pso = dx12u::pipeline_state_object{ dev, rs, vs, ps };
            D3D12_BLEND_DESC bd = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
            bd.RenderTarget[0].BlendEnable = true;
            bd.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            bd.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
            pso.set_blend_states(bd);
            pso.set_depth_test(false);
            pso.commit();
        }
    }

    void text_2d::add_text(std::size_t frame_id, float x, float y, float char_w, float char_h, std::string const& str)
    {
        assert(is_valid());
        assert(frame_id < words.size() && !words.empty());
        assert(words[frame_id].size() < max_num_word);

        word w = 
        {
            x, y,
            char_w * 2.f, char_h * 2.0f,
            str
        };

        words[frame_id].push_back(w);
    }

    void text_2d::clear(std::size_t frame_id)
    {
        assert(is_valid());
        assert(frame_id < words.size() && !words.empty());
        words[frame_id].resize(0);
    }

    void text_2d::draw(std::size_t frame_id, ID3D12GraphicsCommandList* cl)
    {
        assert(is_valid());
        assert(frame_id < words.size() && !words.empty());

        /*
        cl->IASetVertexBuffers(0, 0, nullptr);
        cl->IASetIndexBuffer(nullptr);
        cl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        */

        dx12u::bind(cl, pso);
        bind_descriptor_heap(cl, srd_heap.get_com_ptr().Get());
        cl->SetGraphicsRootDescriptorTable(0, srd_heap.get_gpu_handle(0)); // bind texture

        for (size_t i = 0; i < words[frame_id].size(); ++i)
        {
            auto cb_offset = frame_id * max_num_word + i;
            cl->SetGraphicsRootDescriptorTable(1, srd_heap.get_gpu_handle(1 + cb_offset)); // bind cb

            auto& scale_bias = *reinterpret_cast<tml::vec4*>(cb_ptr + cb_offset * get_cb_size(max_word_sz));
            auto* u_start = reinterpret_cast<float*>(reinterpret_cast<uint8_t*>(&scale_bias) + get_u_start_offset());
            scale_bias.x = words[frame_id][i].x;
            scale_bias.y = words[frame_id][i].y;
            scale_bias.z = words[frame_id][i].char_w;
            scale_bias.w = words[frame_id][i].char_h;
            auto const& str = words[frame_id][i].str;
            for (size_t c = 0; c < str.size(); ++c)
            {
                float const char_w = static_cast<float>(global::font_texture_char_w) / static_cast<float>(global::font_texture_width);
                u_start[c] = static_cast<float>(static_cast<size_t>(str[c]) - global::font_texture_bias) * char_w;
            }

            cl->DrawInstanced(6 * static_cast<uint32_t>(str.size()), 1, 0, 0);
        }
    }

    float text_2d::get_char_w(std::size_t window_width)
    {
        return 2.f * static_cast<float>(global::font_texture_char_w) / static_cast<float>(window_width);
    }

    float text_2d::get_char_h(std::size_t window_height)
    {
        return 2.f * static_cast<float>(global::font_texture_height) / static_cast<float>(window_height);
    }

    text_2d::text_2d(text_2d&& t2d)
    {
        max_word_sz = t2d.max_word_sz;
        max_num_word = t2d.max_num_word;
        srd_heap = std::move(t2d.srd_heap);
        font_texture = t2d.font_texture;
        rs = t2d.rs;
        pso = std::move(t2d.pso);
        cb_mem = t2d.cb_mem;
        cb_ptr = t2d.cb_ptr;
        words = std::move(t2d.words);

        t2d.max_word_sz = 0;
        t2d.max_num_word = 0;
        t2d.srd_heap = dx12u::descriptor_heap{};
        t2d.font_texture = nullptr;
        t2d.rs = nullptr;
        t2d.pso = dx12u::pipeline_state_object{};
        t2d.cb_mem = nullptr;
        t2d.cb_ptr = nullptr;
        t2d.words.clear();
    }

    text_2d& text_2d::operator = (text_2d&& t2d)
    {
        if (this == &t2d)
        {
            return *this;
        }

        max_word_sz = t2d.max_word_sz;
        max_num_word = t2d.max_num_word;
        srd_heap = std::move(t2d.srd_heap);
        font_texture = t2d.font_texture;
        rs = t2d.rs;
        pso = std::move(t2d.pso);
        cb_mem = t2d.cb_mem;
        cb_ptr = t2d.cb_ptr;
        words = std::move(t2d.words);

        t2d.max_word_sz = 0;
        t2d.max_num_word = 0;
        t2d.srd_heap = dx12u::descriptor_heap{};
        t2d.font_texture = nullptr;
        t2d.rs = nullptr;
        t2d.pso = dx12u::pipeline_state_object{};
        t2d.cb_mem = nullptr;
        t2d.cb_ptr = nullptr;
        t2d.words.clear();

        return *this;
    }
    

} // namespace dx12u



