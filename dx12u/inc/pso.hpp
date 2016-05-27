
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
* @file      pso.hpp
* @brief     dx12 pso utilities
*/

#ifndef DX12UTIL_PSO_HPP
#define DX12UTIL_PSO_HPP

#include <root_signature.hpp>
#include <shader.hpp>
#include <cmd_mgr.hpp>
#include <mutex>

namespace dx12u
{

    /**
    * @brief input layout
    */
    using input_layout_list = std::vector<D3D12_INPUT_ELEMENT_DESC>;

    /**
    * @brief pso generator
    */
    class pipeline_state_object
    {
        // TODO add helpers to better configure the pso

    public:

        using pso_com_ptr = com_ptr<ID3D12PipelineState>;

    private:

        device dev{ nullptr };

        pso_com_ptr pso{ nullptr };
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};

        // keep copies of blobs shared ptr to avoid leaks
        root_signature rs_copy{ nullptr };
        shader_blob vs_copy{};
        shader_blob hs_copy{};
        shader_blob ds_copy{};
        shader_blob gs_copy{};
        shader_blob ps_copy{};
        input_layout_list ill_copy{};

        // the pso is created lazily in get_com_ptr() and this is a hassle when multiple threads use the pso
        std::mutex mux;
    public:

        /**
        * @brief create an invalid pso
        */
        pipeline_state_object()
        {}

        /**
        * @brief create a pso with default states
        * @param d device
        * @param rs root signature
        * @param vs vertex shader
        */
        explicit pipeline_state_object(device const& d, root_signature const& rs, shader_blob const& vs)
            : pipeline_state_object{ d, input_layout_list {}, rs, vs }
        {}

        /**
        * @brief create a pso with default states
        * @param d device
        * @param rs root signature
        * @param vs vertex shader
        * @param ps pixel shader
        */
        explicit pipeline_state_object(device const& d, root_signature const& rs, shader_blob const& vs, shader_blob const& ps)
            : pipeline_state_object{ d, input_layout_list{}, rs, vs, ps }
        {}

        /**
        * @brief create a pso with default states
        * @param d device
        * @param rs root signature
        * @param vs vertex shader
        * @param gs geometry shader
        * @param ps pixel shader
        */
        explicit pipeline_state_object(device const& d, root_signature const& rs, shader_blob const& vs, shader_blob const& gs, shader_blob const& ps)
            : pipeline_state_object{ d, input_layout_list{}, rs, vs, gs, ps }
        {}

        /**
        * @brief create a pso with default states
        * @param d device
        * @param rs root signature
        * @param vs vertex shader
        * @param hs hull shader
        * @param ds domain shader
        * @param ps pixel shader
        */
        explicit pipeline_state_object(device const& d, root_signature const& rs, shader_blob const& vs, shader_blob const& hs, shader_blob const& ds, shader_blob const& ps)
            : pipeline_state_object{ d, input_layout_list{}, rs, vs, hs, ds, ps }
        {}

        /**
        * @brief create a pso with default states
        * @param d device
        * @param rs root signature
        * @param vs vertex shader
        * @param hs hull shader
        * @param ds domain shader
        * @param gs geometry shader
        * @param ps pixel shader
        */
        explicit pipeline_state_object(device const& d, root_signature const& rs, shader_blob const& vs, shader_blob const& hs, shader_blob const& ds, shader_blob const& gs, shader_blob const& ps)
            : pipeline_state_object{ d, input_layout_list{}, rs, vs, hs, ds, gs, ps }
        {}

        /**
        * @brief create a pso with default states
        * @param d device
        * @param ill input layout
        * @param rs root signature
        * @param vs vertex shader
        */
        explicit pipeline_state_object(device const& d, input_layout_list const& ill,
            root_signature const& rs, shader_blob const& vs)
            : pipeline_state_object{ d, ill, rs, vs, shader_blob{} }
        {}

        /**
        * @brief create a pso with default states
        * @param d device
        * @param ill input layout
        * @param rs root signature
        * @param vs vertex shader
        * @param ps pixel shader
        */
        explicit pipeline_state_object(device const& d, input_layout_list const& ill,
            root_signature const& rs, shader_blob const& vs, shader_blob const& ps)
            : pipeline_state_object{ d, ill, rs, vs, shader_blob{}, ps }
        {}

        /**
        * @brief create a pso with default states
        * @param d device
        * @param ill input layout
        * @param rs root signature
        * @param vs vertex shader
        * @param gs geometry shader
        * @param ps pixel shader
        */
        explicit pipeline_state_object(device const& d, input_layout_list const& ill,
            root_signature const& rs, shader_blob const& vs, shader_blob const& gs, shader_blob const& ps)
            : pipeline_state_object{ d, ill, rs, vs, shader_blob{}, shader_blob{}, gs, ps }
        {}

        /**
        * @brief create a pso with default states
        * @param d device
        * @param ill input layout
        * @param rs root signature
        * @param vs vertex shader
        * @param hs hull shader
        * @param ds domain shader
        * @param ps pixel shader
        */
        explicit pipeline_state_object(device const& d, input_layout_list const& ill,
            root_signature const& rs, shader_blob const& vs, shader_blob const& hs, shader_blob const& ds, shader_blob const& ps)
            : pipeline_state_object{ d, ill, rs, vs, hs, ds, shader_blob{}, ps }
        {}

        /**
        * @brief create a pso with default states
        * @param d device
        * @param ill input layout
        * @param rs root signature
        * @param vs vertex shader
        * @param hs hull shader
        * @param ds domain shader
        * @param gs geometry shader
        * @param ps pixel shader
        */
        explicit pipeline_state_object(device const& d, input_layout_list const& ill,
            root_signature const& rs, shader_blob const& vs, shader_blob const& hs, shader_blob const& ds, shader_blob const& gs, shader_blob const& ps);

        /**
        * @brief non copyable
        */
        pipeline_state_object(pipeline_state_object const&) = delete;

        /**
        * @brief non copy-assignable
        */
        pipeline_state_object& operator = (pipeline_state_object const&) = delete;

        /**
        * @brief move a pso. the moved from object becomes invalid
        * @param other pso to move from
        */
        pipeline_state_object(pipeline_state_object&& other) noexcept;

        /**
        * @brief move assign a pso. the moved from object becomes invalid
        * @param other pso to move from
        */
        pipeline_state_object& operator = (pipeline_state_object&& other) noexcept;

        /**
        * @brief set gpu nodes mask
        * @param mask bits set to 1 for GPU nodes that use the pso
        * @note the update invalidates the pso
        */
        void set_gpu_node_mask(std::uint32_t mask);

        /**
        * @brief set depth stencil states
        * @param dss depth stencil states desc
        * @note the update invalidates the pso
        */
        void set_depth_stencil_states(D3D12_DEPTH_STENCIL_DESC const& dss);

        /**
        * @brief set blend states
        * @param bs blend states desc
        * @note the update invalidates the pso
        */
        void set_blend_states(D3D12_BLEND_DESC const& bs);

        /**
        * @brief set the cull mode
        * @param mode cull mode
        * @note the update invalidates the pso
        */
        void set_cull_mode(D3D12_CULL_MODE mode);

        /**
        * @brief set front ccw
        * @param ccw true:ccw; false:cw
        * @note the update invalidates the pso
        */
        void set_ccw(bool ccw);

        /**
        * @brief set the fill mode
        * @param mode fill mode
        * @note the update invalidates the pso
        */
        void set_fill_mode(D3D12_FILL_MODE mode);

        /**
        * @brief enable/disable depth test
        * @param state true enables depth test; false disables it
        * @note the update invalidates the pso
        */
        void set_depth_test(bool state);

        /**
        * @brief enable/disable depth writes
        * @param state true enables depth writes; false disables it
        * @note the update invalidates the pso
        */
        void set_depth_write(bool state);

        /**
        * @brief set depth comparison function
        * @param f true comparison function
        * @note the update invalidates the pso
        */
        void set_depth_func(D3D12_COMPARISON_FUNC f);

        /**
        * @brief set depth format
        * @param fmt format
        * @note the update invalidates the pso
        */
        void set_depth_format(DXGI_FORMAT fmt);

        /**
        * @brief access the underlying pso com ptr
        * @return pso com ptr
        * @note the ptr is created (lazy creation) if the pso is invalid
        */
        pso_com_ptr const& get_com_ptr();

        /**
        * @brief commit a pso
        * @note forces the pso creation (otherwise creation is lazy)
        */
        void commit()
        {
            get_com_ptr();
        }

        /**
        * @brief check whether the pso is valid of not
        * @return true if the pso is valid; false otherwise
        */
        bool is_valid() const noexcept
        {
            return vs_copy.is_valid() && rs_copy.Get() != nullptr;
        }

        /**
        * @brief destroy the pso but keep its data
        */
        void nullify()
        {
            pso = nullptr;
        }

        /**
        * @brief access the pso root signature
        * @return root signature
        */
        root_signature const& get_root_signature() const noexcept
        {
            return rs_copy;
        }

        /**
        * @brief set the render target format
        * @param format render target format
        * @param rt_index render target index (0 to 7)
        */
        void set_rt_format(DXGI_FORMAT format, std::size_t rt_index = 0);

        /**
        * @brief explicitly release blobs
        */
        void release_blob();
    };

    /**
    * @brief bind a pso
    * @param cl command list in which the bind is recorded
    * @param pso the pso to bind
    * @note the pso is passed by reference as it can be lazily created
    * @note the behavior is undefined if the pso is invalid
    */
    void bind(ID3D12GraphicsCommandList* cl, pipeline_state_object& pso);

    /**
    * @brief bind a pso
    * @param cl command list in which the bind is recorded
    * @param pso the pso to bind
    * @param rs the root signature associeted to the pso
    */
    void bind(ID3D12GraphicsCommandList* cl, ID3D12RootSignature* rs, ID3D12PipelineState* pso);

    /**
    * @brief get default input layout (float3:POSITION, float3:NORMAL, float3:TANGENT, float2:TEXCOORD)
    * @return default input layout
    */
    input_layout_list get_default_input_layout();

} // namespace dx12u



#endif // DX12UTIL_PSO_HPP
