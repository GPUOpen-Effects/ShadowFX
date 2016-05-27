
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


#include <app.hpp>
#include <swap_chain.hpp>
#include <descriptor_heap.hpp>
#include <pso.hpp>
#include <depth_stencil.hpp>
#include <vertex_buffer.hpp>
#include <render_target.hpp>
#include <texture.hpp>
#include <iostream>
#include <future>
#include <functional>
#include <memory>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iterator>
#include <gu/gu.hpp>
// #include <glm/geometric.hpp>
// #include <glm/gtc/matrix_transform.hpp>
#include <tml/mat.hpp>
#include <queries.hpp>
#include <text.hpp>

#include <AMD_ShadowFX.h>

#include "mesh_loader.hpp"
#include "constant_buffer.hpp"


/////////////////////////////////////////////////////////////////
// typedefs
/////////////////////////////////////////////////////////////////

using std::size_t;
using std::uint32_t;
using std::uint64_t;
using vbo_list = std::vector<dx12u::vbo>;


/////////////////////////////////////////////////////////////////
// sample parameters
/////////////////////////////////////////////////////////////////

struct settings
{
    // window settings
    bool         is_windowed = true;
    size_t       width = 1920;
    size_t       height = 1080;

    // shadow fx settings
    size_t       shadow_res = 2048;
    size_t       filtersz = 7;
    std::string  fetch = "gather";
    std::string  filter = "uniform";
    std::string  tap = "fixed";
    float        depth_bias = 0.f;
} g_setting;



/////////////////////////////////////////////////////////////////
// shadow_fx_sample holds the implementation details
/////////////////////////////////////////////////////////////////

class shadow_fx_sample
{
    /////////////////////////////////////////////////////////////////
    // shadow sample member variables
    /////////////////////////////////////////////////////////////////

    // the sample buffers commands for multiple frames
    // note that resources that get updated in each frame will be duplicated
    // such resources will be stored in an array of m_num_buffered_frame elements
    static size_t const            m_num_buffered_frame = 3; // controls how many frames get buffered
    size_t                         m_frame_id = 0; // increments every frame. This is used when setting frame fences

    // device
    // dx12u::device is an alias of com_ptr<ID3D12Device>
    // create_device initializes a device for the first dx12 capable hardware it finds
    dx12u::device                  m_dev = dx12u::create_device();

    // command queue
    // dx12u::cmd_queue wraps a ID3D12CommandQueue and gives functions to control fences associeted with the m_queue
    dx12u::cmd_queue               m_queue{ m_dev };

    // swap chain
    // DXGI_SWAP_EFFECT_FLIP_DISCARD will be used with m_num_buffered_frame buffers in the chain
    dx12u::swap_chain              m_swp_chain{}; // the swap chain
    dx12u::resource                m_frame_buffer[m_num_buffered_frame]; // frame buffers linked to m_swp_chain
    bool                           m_is_fullscreen = false; // keep track of the swap chain windowed/fullscreen state

    // command allocators lists
    // a seperate allocator is used for each buffered frame
    dx12u::cmd_allocator           m_cmd_allocator[m_num_buffered_frame];
    dx12u::gfx_cmd_list            m_cmd_list[m_num_buffered_frame];

    // view depth buffer: TEX2D FLOAT_32D
    dx12u::resource                m_view_space_db[m_num_buffered_frame]; // view space depth buffer

    // light depth buffer: TEX2D_ARRAY FLOAT_32D. One array layer for each point light cube face
    dx12u::resource                m_light_space_db[m_num_buffered_frame]; // light space depth buffer texture array

    // the m_shadow_mask buffer is the result of shadow_fx
    // shadow_fx fills m_shadow_mask based on the input view space and light depth buffers
    // m_shadow_mask has the same resolution as the screen and gives for each pixel a value between black and white
    // black means the pixel is in shadow and white means the pixel is completely lit
    dx12u::resource                m_shadow_mask[m_num_buffered_frame]; // shadow mask buffer

    // mesh vertex buffer object
    // a mesh can be composed of sub meshes
    // vbo_list is a list of vbo. a vbo contains a vertex and index buffer resources as well as their views
    vbo_list                       m_mesh_vbo{}; // mesh vertex/index buffer

    // meshes rendered in the sample
    mesh_list                      m_meshes = load_mesh("../media/conference/conference.obj"); // conference meshe
    size_t                         m_num_mesh = m_meshes.size(); // alias for m_meshes.size()

    // constant buffer resource
    // for simplicity all constant buffers are stored in one gpu allocation
    // to further simplify addressing each constant buffer uses one page (4k space) in this allocation
    dx12u::resource                m_const_buffer_mem{};

    // plastic_ps_cb is the constant buffer structure used for shading
    // each rendered mesh can have a different material
    // plastic_ps_cb_list is list of plastic_ps_cb structures: one per material
    using plastic_ps_cb_list = std::vector<plastic_ps_cb*>;

    // cpu mapped constant buffers
    // each constant buffer points to a page in m_const_buffer_mem
    // mesh_plastic_ps_cb is an array of plastic_ps_cb; one per mesh
    // the other constant buffers are global for all the meshes
    plastic_ps_cb_list             m_mesh_plastic_ps_cb[m_num_buffered_frame]; // one page per mesh in m_const_buffer_mem: page[0] to page[m_num_mesh - 1]
    plastic_vs_cb*                 m_mesh_plastic_vs_cb[m_num_buffered_frame] = { nullptr }; // one page page[m_num_mesh]
    depth_pass_cb*                 m_mesh_depth_pass_cb[m_num_buffered_frame][1 + NUM_CUBE_FACE] = { nullptr }; // the first constant buffer page[m_num_mesh+1] is for view depth 
                                                                                                            // the next NUM_CUBE_FACE for light depth: page[m_num_mesh+2] to page[m_num_mesh + 2 + NUM_CUBE_FACE - 1]
    size_t                         m_num_cb_page = m_num_mesh + 2 + NUM_CUBE_FACE; // total number of constant buffers (per frame)

    // render targets descriptor heap
    // the sample uses 2 render targets in one frame: m_frame_buffer and m_shadow_mask
    // a seperate render target exist for each buffered frame
    // m_frame_buffer[m_num_buffered_frame] descriptors are stores in the first m_num_buffered_frame slots of the heap
    // m_shadow_mask[m_num_buffered_frame] descriptors are stores in the seconds set of m_num_buffered_frame slots
    size_t const                   m_num_rtv = 2 * m_num_buffered_frame; // number of views in m_rtv_heap: m_num_buffered_frame for m_frame_buffer and m_num_buffered_frame for m_shadow_mask
    dx12u::descriptor_heap         m_rtv_heap{ m_dev.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_num_rtv }; // rtv descriptor heap

    // depth stencil descriptor heap
    // the first slot of the heap is used for the view space depth buffer
    // the next NUM_CUBE_FACE slots are used for each light space depth buffer cube face
    // subsequent slots are used for the other buffered frames with the same layout as frame 0
    size_t const                   m_num_dsv_per_frame = NUM_CUBE_FACE + 1; // number of descriptors per frame
    size_t const                   m_num_dsv = m_num_buffered_frame * m_num_dsv_per_frame; // total number of descriptors
    dx12u::descriptor_heap         m_dsv_heap{ m_dev.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_num_dsv }; // depth-stencil descriptor heap

    // shader resource descriptor heap
    // the srd heap stores descriptors for all constant buffers and textures
    // 2 * m_num_mesh are reserved to m_mesh_plastic_ps_cb and m_mesh_plastic_vs_cb CBVs (a CBV pair per mesh)
    // the next slot is used for m_mesh_depth_pass_cb CBV associated with the view depth pass
    // the next NUM_CUBE_FACE slots are used for m_mesh_depth_pass_cb CBV associated with the light depth passes
    // the next NUM_CUBE_FACE slots are used for light depth SRVs. One SRV per cube face
    // the next slot is used for view depth SRV
    // the last slot is used for m_shadow_mask SRV
    // the exact layout is
    // slot 0 to  2*m_num_mesh-1 : m_mesh_plastic_ps_cb and m_mesh_plastic_vs_cb cbv pair per mesh
    // slot 2*m_num_mesh to 2*m_num_mesh + (NUM_CUBE_FACE+1) - 1: m_mesh_depth_pass_cb cbv
    // slot 2*m_num_mesh + (NUM_CUBE_FACE+1) to 2*m_num_mesh + (NUM_CUBE_FACE+1) + NUM_CUBE_FACE-1: m_light_space_db srv
    // slot 2*m_num_mesh + 2 * NUM_CUBE_FACE + 1: m_view_space_db srv
    // slot 2*m_num_mesh + 2 * NUM_CUBE_FACE + 2: m_shadow_mask srv
    // the following variables with the suffix _offset help indexing the heap
    size_t const                   m_mesh_depth_pass_cb_offset = 2 * m_num_mesh;
    size_t const                   m_light_space_db_offset = 2 * m_num_mesh + (NUM_CUBE_FACE + 1);
    size_t const                   m_view_space_db_offset = 2 * m_num_mesh + 2 * NUM_CUBE_FACE + 1;
    size_t const                   m_sh_mask_offset = 2 * m_num_mesh + 2 * NUM_CUBE_FACE + 2;
    size_t const                   m_num_srd_per_frame = m_sh_mask_offset + 1; // number of srd per frame
    size_t const                   m_num_srd = m_num_srd_per_frame * m_num_buffered_frame; // total number of srd
    dx12u::descriptor_heap         m_srd_heap{ m_dev.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_num_srd }; // srd heap

    // root signatures
    // dx12u::root_signature is simply an alias for com_ptr<ID3D12RootSignature>
    // there is one root signature for each rendering pass: the depth pass and the color pass
    dx12u::root_signature          m_depth_pass_rs{ nullptr }; // b0 in root
    dx12u::root_signature          m_color_pass_rs{ nullptr }; // descriptor table with b0 and t0

    // pso
    // there is one pso for each rendering pass: the depth pass and the color pass
    dx12u::pipeline_state_object   m_depth_pass_pso{};
    dx12u::pipeline_state_object   m_color_pass_pso{};

    // the structure ShadowFX_Desc contains the parameter description input to shadow_fx
    // one instance is kept for each buffered frame
    AMD::ShadowFX_Desc             m_shadow_desc[m_num_buffered_frame];

    // the sample has a single point light
    tml::vec3                      m_light_pos = tml::vec3(4.f, 5.f, 0.f); // light position
    tml::vec4                      m_light_col = tml::vec4(tml::vec3(.15f), 1.f); // light color

    // camera object
    gu::free_camera                m_camera{ tml::vec3(-26.f, 5.f, 12.f) };
    gu::elapsed                    m_elapse{}; // stores the time elapsed since the last frame execution
    float                          m_elapsed_since_begin = 0.f; // stores the time elapsed since the start of the sample

    // debug text displayed on screen
    bool                           m_enable_text = true; // used to show/hide the text
    dx12u::text_2d                 m_ui_text{ m_dev, 32, 16, m_num_buffered_frame, false }; // the text object

    // simple fps counter
    gu::timer                      m_second_counter{}; // timer reset every second
    size_t                         m_frame_counter = 0; // frame count per second
    size_t                         m_fps = 0; // simple fps value

    // the light is animated. This variable allows to pause the animation
    bool                           m_pause = false;

    // light camera debugging controls
    // the key L allows to switch to different point light views
    bool                           m_enable_debug = false; // when true the light point of view is used as camera
    size_t                         m_light_debug_id = 0; // the cube face id for which the camera is set

public:

    /////////////////////////////////////////////////////////////////
    // shadow sample initialization
    /////////////////////////////////////////////////////////////////

    shadow_fx_sample()
    {
        // the function create_pso compiles shaders and PSO objects
        // PSO creation can be slow. It's generally recommended to create PSOs using seperate threads
        // create_pso is executed asynchroniously although the used PSOs are simple
        auto pso_future = std::async(std::launch::async, &shadow_fx_sample::create_pso, this);

        // create command allocators and lists
        // A seperate allocator is created per buffered frame
        for (size_t frame_lid = 0; frame_lid < m_num_buffered_frame; ++frame_lid)
        {
            m_cmd_allocator[frame_lid] = dx12u::cmd_allocator{ m_dev };
            m_cmd_list[frame_lid] = m_cmd_allocator[frame_lid].alloc();
            auto r = m_cmd_list[frame_lid]->Close();
            dx12u::throw_if_error(r);
        }

        // shadow_fx library initialization
        init_shadow_fx();
        
        // constant buffers initialization
        create_const_buffers();

        // vertex buffers initialization
        create_vertex_buffers();
        // once vertex buffer resources are created there is no need to keep the cpu memory allocated for the mesh
        m_meshes.clear(); // free the cpu memory allocated for the mesh

        // light depth buffer resource creation
        create_shadow_map_resources();

        // create the swap chain
        // view dependent resources are created in update_swap_chain
        update_swap_chain();

        // wait for pso creation to finish
        pso_future.get();
    }


    /////////////////////////////////////////////////////////////////
    // shadow_fx library initialization
    /////////////////////////////////////////////////////////////////

    void init_shadow_fx()
    {
        // shadow_fx is parameterized using the object m_shadow_desc
        // shadow_fx builds a shadow mask that gives the intensity of each pixel
        // because we buffer frames we have to build m_num_buffered_frame different masks
        // m_num_buffered_frame independent invocations of the library are used to build the different mask
        // the different invocations can have different parameters and because of that m_shadow_desc is an array of m_num_buffered_frame elements
        for (size_t frame_lid = 0; frame_lid < m_num_buffered_frame; ++frame_lid)
        {
            // m_MaxInstance is the maximum number of shadow masks that can be built  (in parallel)
            // in this sample m_num_buffered_frame instances are used. One instance is used per frame
            m_shadow_desc[frame_lid].m_MaxInstance = static_cast<uint32_t>(m_num_buffered_frame);

            // the shadow library uses the D3D12 device to create its own constant buffers and PSOs
            m_shadow_desc[frame_lid].m_pDevice = m_dev.Get();

            // the other parameters are the same used in the DX11 version of shadow_fx
            m_shadow_desc[frame_lid].m_ActiveLightCount = NUM_CUBE_FACE;
            m_shadow_desc[frame_lid].m_Execution = AMD::SHADOWFX_EXECUTION_UNION;
            m_shadow_desc[frame_lid].m_TextureFetch = g_setting.fetch == "gather" ? AMD::SHADOWFX_TEXTURE_FETCH_GATHER4 : AMD::SHADOWFX_TEXTURE_FETCH_PCF;
            m_shadow_desc[frame_lid].m_FilterSize = static_cast<AMD::SHADOWFX_FILTER_SIZE>(g_setting.filtersz);
            m_shadow_desc[frame_lid].m_Filtering = g_setting.filter == "uniform" ? AMD::SHADOWFX_FILTERING_UNIFORM : AMD::SHADOWFX_FILTERING_CONTACT;
            m_shadow_desc[frame_lid].m_TapType = g_setting.tap == "fixed" ? AMD::SHADOWFX_TAP_TYPE_FIXED : AMD::SHADOWFX_TAP_TYPE_POISSON;
            m_shadow_desc[frame_lid].m_TextureType = AMD::SHADOWFX_TEXTURE_2D_ARRAY;
            m_shadow_desc[frame_lid].m_OutputFormat = DXGI_FORMAT_R8_UNORM;

            // a texture array is used to store the shadow depth map
            unsigned seq[] = { 0, 1, 2, 3, 4, 5 };
            std::copy(std::begin(seq), std::end(seq), m_shadow_desc[frame_lid].m_ArraySlice);

            // initial depth and normal bias values
            g_setting.depth_bias = .0005f * 8.f / (g_setting.shadow_res / 512.f);
            float normal_bias = .001f * 8.f / (g_setting.shadow_res / 512.f);

            std::fill(std::begin(m_shadow_desc[frame_lid].m_DepthTestOffset), std::end(m_shadow_desc[frame_lid].m_DepthTestOffset), g_setting.depth_bias);
            std::fill(std::begin(m_shadow_desc[frame_lid].m_NormalOffsetScale), std::end(m_shadow_desc[frame_lid].m_NormalOffsetScale), normal_bias);

            // we enable normal bias and instruct the library to compute the normal from depth rather than providing a normal map
            m_shadow_desc[frame_lid].m_NormalOption = AMD::SHADOWFX_NORMAL_OPTION_CALC_FROM_DEPTH;

            for (size_t i = 0; i < NUM_CUBE_FACE; ++i)
            {
                // m_SunArea is only used for contact hardening shadows
                m_shadow_desc[frame_lid].m_SunArea[i] = 8.f;

                // m_ShadowRegion is used when an atlas texture is used. This sample is limited to a texture array and the region is set to the whole texture in each slice
                m_shadow_desc[frame_lid].m_ShadowRegion[i].x = 0.f;
                m_shadow_desc[frame_lid].m_ShadowRegion[i].y = 0.f;
                m_shadow_desc[frame_lid].m_ShadowRegion[i].z = 1.f;
                m_shadow_desc[frame_lid].m_ShadowRegion[i].w = 1.f;
            }
        }

        // this function initializes the library: mainly allocating constant buffers and PSOs
        auto sh_err = AMD::ShadowFX_Initialize(m_shadow_desc[0]);
        process_shadow_fx_error(sh_err);
    }


    /////////////////////////////////////////////////////////////////
    // view dependent resources initialization
    /////////////////////////////////////////////////////////////////

    void create_view_dependent_resources()
    {
        // view dependent resources exist separately for each buffered frame
        for (size_t frame_lid = 0; frame_lid < m_num_buffered_frame; ++frame_lid)
        {
            create_view_dependent_resources(frame_lid);
        }
    }

    void create_view_dependent_resources(size_t frame_lid)
    {
        // the view space render targets resolution will be the same as the window size
        auto w = m_swp_chain.get_size().x;
        auto h = m_swp_chain.get_size().y;

        // m_shadow_mask creation. DXGI_FORMAT_R8_UNORM is usually enough for the mask
        auto sh_rt = dx12u::make_render_target(m_dev, w, h, DXGI_FORMAT_R8_UNORM);
        m_shadow_mask[frame_lid] = sh_rt.rt_rsrc;

        // create a RTV and SRV views for m_shadow_mask. The declaration of m_rtv_heap and m_srd_heap describe what heap slots are reserved to m_shadow_mask
        m_dev->CreateRenderTargetView(m_shadow_mask[frame_lid].Get(), &sh_rt.rtv_desc, m_rtv_heap.get_cpu_handle(m_num_buffered_frame + frame_lid));
        m_dev->CreateShaderResourceView(m_shadow_mask[frame_lid].Get(), &sh_rt.srv_desc, m_srd_heap.get_cpu_handle(frame_lid * m_num_srd_per_frame + m_sh_mask_offset));

        // view depth buffer. DXGI_FORMAT_D32_FLOAT is used as the stencil is not needed in this sample
        auto vdb = dx12u::make_depth_stencil(m_dev, w, h, DXGI_FORMAT_D32_FLOAT);

        // create a DSV and SRV views for m_view_space_db.The declaration of m_dsv_heap and m_srd_heap describe what heap slots are reserved to m_view_space_db
        m_view_space_db[frame_lid] = vdb.db_rsrc;
        m_dev->CreateDepthStencilView(m_view_space_db[frame_lid].Get(), &vdb.dbv_desc[0], m_dsv_heap.get_cpu_handle(frame_lid * m_num_dsv_per_frame + 0));
        m_dev->CreateShaderResourceView(m_view_space_db[frame_lid].Get(), &vdb.srv_desc, m_srd_heap.get_cpu_handle(frame_lid * m_num_srd_per_frame + m_view_space_db_offset));

        // shadow_fx used the view space depth buffer as input
        // the depth buffer, it's size and view are stored in m_shadow_desc structure
        m_shadow_desc[frame_lid].m_pDepth = m_view_space_db[frame_lid].Get();
        m_shadow_desc[frame_lid].m_DepthSRV = vdb.srv_desc;
        m_shadow_desc[frame_lid].m_DepthSize.x = static_cast<float>(w);
        m_shadow_desc[frame_lid].m_DepthSize.y = static_cast<float>(h);
    }


    /////////////////////////////////////////////////////////////////
    // shadow map depth resources initialization
    /////////////////////////////////////////////////////////////////

    void create_shadow_map_resources()
    {
        // like view dependent resources, seperate shadow maps are needed for seperate buffered frames
        for (size_t frame_lid = 0; frame_lid < m_num_buffered_frame; ++frame_lid)
        {
            create_shadow_map_resources(frame_lid);
        }
    }

    void create_shadow_map_resources(size_t frame_lid)
    {
        // shadow depth buffer texture array. DXGI_FORMAT_D32_FLOAT is used because we don't need stencil
        auto ldb = dx12u::make_depth_stencil(m_dev, g_setting.shadow_res, g_setting.shadow_res, NUM_CUBE_FACE, DXGI_FORMAT_D32_FLOAT);
        m_light_space_db[frame_lid] = ldb.db_rsrc;

        // and DSV and SRV view is created for each layer of the depth texture array
        for (size_t i = 0; i < NUM_CUBE_FACE; ++i)
        {
            m_dev->CreateDepthStencilView(m_light_space_db[frame_lid].Get(), &ldb.dbv_desc[i], m_dsv_heap.get_cpu_handle(frame_lid * m_num_dsv_per_frame + i + 1));
            auto v = ldb.srv_desc;
            v.Texture2DArray.FirstArraySlice = static_cast<uint32_t>(i);
            v.Texture2DArray.ArraySize = 1;
            m_dev->CreateShaderResourceView(m_light_space_db[frame_lid].Get(), &v, m_srd_heap.get_cpu_handle(frame_lid * m_num_srd_per_frame + m_light_space_db_offset + i));
        }

        // set light depth data in shadow_fx
        m_shadow_desc[frame_lid].m_pShadow = m_light_space_db[frame_lid].Get();
        m_shadow_desc[frame_lid].m_ShadowSRV = ldb.srv_desc;
        for (size_t i = 0; i < NUM_CUBE_FACE; ++i)
        {
            m_shadow_desc[frame_lid].m_ShadowSize[i].x = static_cast<float>(g_setting.shadow_res);
            m_shadow_desc[frame_lid].m_ShadowSize[i].y = static_cast<float>(g_setting.shadow_res);
        }
    }


    /////////////////////////////////////////////////////////////////
    // vertex buffer initialization
    /////////////////////////////////////////////////////////////////

    void create_vertex_buffers()
    {
        // the command allocator and list are reset and will be used to upload mesh data
        m_cmd_allocator[0].safe_reset();
        auto r = m_cmd_list[0]->Reset(m_cmd_allocator[0].get_com_ptr().Get(), nullptr);
        dx12u::throw_if_error(r);

        // some basic mesh pre-processing
        auto mesh_transform = tml::translate(tml::mat4{}, tml::vec3(-4.f, -1.f, 0.f)) * tml::scale(tml::mat4{}, tml::vec3(30.f, 30.f, 30.f));
        for (auto& m : m_meshes)
        {
            gu::transform(m.vbo, mesh_transform);
        }

        // create a list of vertex buffer objects. One per sub-mesh
        m_mesh_vbo.resize(m_meshes.size());
        std::vector<dx12u::vb_resources> mesh_resources(m_meshes.size());
        for (size_t i = 0; i < m_meshes.size(); ++i)
        {
            // skip potential empty meshes
            if (m_meshes[i].vbo.vb.empty())
            {
                continue;
            }

            // dx12u::make_vb creates a vertex buffer and uploads its data using m_cmd_list[0] to push gpu commands
            mesh_resources[i] = dx12u::make_vb(m_dev, m_cmd_list[0], m_meshes[i].vbo);
            m_mesh_vbo[i].vb = mesh_resources[i].vb;
            m_mesh_vbo[i].ib = mesh_resources[i].ib;
            m_mesh_vbo[i].vbv = mesh_resources[i].vb_view;
            m_mesh_vbo[i].ibv = mesh_resources[i].ib_view;
        }

        // finalize and execute m_cmd_list
        r = m_cmd_list[0]->Close(); // the list is open at creation
        dx12u::throw_if_error(r);
        m_queue.push(m_cmd_list[0]);
        // wait before releasing temporary upload buffers
        m_queue.sync();
    }


    /////////////////////////////////////////////////////////////////
    // constant buffers initialization
    /////////////////////////////////////////////////////////////////

    void create_const_buffers()
    {
        // a single buffer is allocated in which various constant buffers are sub-allocated
        // one constant buffer is sub-allocated as one memory page
        size_t const psz = dx12u::get_page_size();

        // m_num_cb_page constant buffers are used. each buffer is duplicated for each buffered frame (think renaming)
        // since each constant buffer is one page in memory the total size is psz * m_num_cb_page * m_num_buffered_frame
        size_t const total_cb_size_per_frame = psz * m_num_cb_page;
        size_t const total_cb_size = total_cb_size_per_frame * m_num_buffered_frame;

        // gpu memory allocation: m_const_buffer_mem
        auto r = m_dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(total_cb_size), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_const_buffer_mem));
        dx12u::throw_if_error(r);

        // map m_const_buffer_mem
        uint8_t* cb_global_ptr = nullptr;
        r = m_const_buffer_mem->Map(0, nullptr, reinterpret_cast<void**>(&cb_global_ptr));
        dx12u::throw_if_error(r);
        assert(!!cb_global_ptr);

        // the following code computes the cpu pointer of each command buffer
        // the code also creates a view for each constant buffer
        for (size_t frame_lid = 0; frame_lid < m_num_buffered_frame; ++frame_lid)
        {
            // pointer to the first constant buffer in a frame
            auto frame_cb_ptr = cb_global_ptr + frame_lid * total_cb_size_per_frame;

            // set the pointer m_mesh_plastic_ps_cb
            m_mesh_plastic_ps_cb[frame_lid].resize(m_num_mesh);
            for (size_t m = 0; m < m_num_mesh; ++m)
            {
                m_mesh_plastic_ps_cb[frame_lid][m] = reinterpret_cast<plastic_ps_cb*>(frame_cb_ptr + m * psz); // page[0] -> page[m_num_mesh-1]
            }

            // set the pointer m_mesh_plastic_vs_cb
            m_mesh_plastic_vs_cb[frame_lid] = reinterpret_cast<plastic_vs_cb*>(frame_cb_ptr + m_num_mesh * psz); // page[m_num_mesh]

            // set the pointer m_mesh_depth_pass_cb
            for (size_t i = 0; i < 1 + NUM_CUBE_FACE; ++i)
            {
                m_mesh_depth_pass_cb[frame_lid][i] = reinterpret_cast<depth_pass_cb*>(frame_cb_ptr +
                    (m_num_mesh + 1 + i) * psz); // page[m_num_mesh+1] -> page[m_num_mesh+1 + NUM_CUBE_FACE]
            }

            // constant buffer views
            D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc = {};
            cbv_desc.SizeInBytes = static_cast<uint32_t>(psz);
            for (size_t m = 0; m < m_num_mesh; ++m)
            {
                // m_mesh_plastic_ps_cb view
                cbv_desc.BufferLocation = m_const_buffer_mem->GetGPUVirtualAddress() + frame_lid * total_cb_size_per_frame + m * psz;
                m_dev->CreateConstantBufferView(&cbv_desc, m_srd_heap.get_cpu_handle(frame_lid * m_num_srd_per_frame + 2 * m));

                // m_mesh_plastic_vs_cb view
                cbv_desc.BufferLocation = m_const_buffer_mem->GetGPUVirtualAddress() + frame_lid * total_cb_size_per_frame + m_num_mesh * psz;
                m_dev->CreateConstantBufferView(&cbv_desc, m_srd_heap.get_cpu_handle(frame_lid * m_num_srd_per_frame + 2 * m + 1));
            }

            // m_mesh_depth_pass_cb view
            for (size_t i = 0; i < 1 + NUM_CUBE_FACE; ++i)
            {
                cbv_desc.BufferLocation = m_const_buffer_mem->GetGPUVirtualAddress() + frame_lid * total_cb_size_per_frame + (m_num_mesh + 1 + i) * psz;
                m_dev->CreateConstantBufferView(&cbv_desc, m_srd_heap.get_cpu_handle(frame_lid * m_num_srd_per_frame + 2 * m_num_mesh + i));
            }

            // set m_mesh_plastic_ps_cb static data
            for (size_t i = 0; i < m_num_mesh; ++i)
            {
                tml::vec4 specular = tml::vec4(m_meshes[i].mat.specular, m_meshes[i].mat.shininess);
                tml::vec3 ambient = m_meshes[i].mat.diffuse * .3f; // m_meshes[i].mat.ambient is zero in the used conference mesh
                m_mesh_plastic_ps_cb[frame_lid][i]->set(ambient, m_meshes[i].mat.diffuse, specular, m_light_col);
            }
        }
    }


    /////////////////////////////////////////////////////////////////
    // root signatures initialization
    /////////////////////////////////////////////////////////////////

    void init_root_signatures()
    {
        // depth pass root signature
        {
            // dx12u::descriptor_sig_list is a array of signatures
            dx12u::descriptor_sig_list descriptor_table{};
            descriptor_table.append(dx12u::descriptor_sig{ dx12u::descriptor_type::cbv, 0, 0, dx12u::shader_mask::vs });

            // a root table is created and will have descriptor_table in the first root slot
            dx12u::descriptor_sig_list root_table{};
            root_table.append(descriptor_table);

            // dx12u::make_root_signature serializes and creates the signature
            m_depth_pass_rs = dx12u::make_root_signature(m_dev.Get(), root_table);
        }

        // color pass root signature
        {
            // 2 descriptor tables are used for the color pass
            // the descriptor_table1 used for constant buffers
            // the descriptor_table2 used for texture
            // separating the tables simplifies the SRD heap and allows switching const buffers regardless of the textures
            dx12u::descriptor_sig_list descriptor_table1{};
            dx12u::descriptor_sig_list descriptor_table2{};
            descriptor_table1.append(dx12u::descriptor_sig{ dx12u::descriptor_type::cbv, 0 });
            descriptor_table1.append(dx12u::descriptor_sig{ dx12u::descriptor_type::cbv, 1 });
            descriptor_table2.append(dx12u::descriptor_sig{ dx12u::descriptor_type::srv, 0, 0, dx12u::shader_mask::ps });

            // root
            dx12u::descriptor_sig_list root_table{};
            root_table.append(descriptor_table1);
            root_table.append(descriptor_table2);
            m_color_pass_rs = dx12u::make_root_signature(m_dev.Get(), root_table);
        }
    }


    /////////////////////////////////////////////////////////////////
    // PSOs initialization
    /////////////////////////////////////////////////////////////////

    void create_pso()
    {
        // create the root signatures as they're needed by the PSOs
        init_root_signatures();

        // vertex layout
        // dx12u::get_default_input_layout provides the layout: POSITION (RGB_32F), NORMAL (RGB_32F), TANGENT (RGB_32F), TEXCOORD (RG_32F)
        dx12u::input_layout_list ill = dx12u::get_default_input_layout();

        // the shader shadowed_plastic needs a definition for MAX_LIGHT 
        using macro = std::pair<std::string, std::string>;
        macro max_light = macro{ "MAX_LIGHT", std::to_string(NUM_CUBE_FACE) };
        D3D_SHADER_MACRO def[] =
        {
            max_light.first.c_str(), max_light.second.c_str(),
            nullptr, nullptr
        };

        // shaders compilation
        auto color_pass_vs = dx12u::compile_from_file("../../framework/d3d12/shader/basic_common/shadowed_plastic.hlsl", def, "vs_main", "vs_5_0");
        auto color_pass_ps = dx12u::compile_from_file("../../framework/d3d12/shader/basic_common/shadowed_plastic.hlsl", def, "ps_main", "ps_5_0");
        auto sh_depth_pass_vs = dx12u::compile_from_file("../../framework/d3d12/shader/common/depth_pass.hlsl", nullptr, "vs_main", "vs_5_0");

        // depth pass PSO
        // dx12u::pipeline_state_object sets most states to default
        m_depth_pass_pso = dx12u::pipeline_state_object{ m_dev, ill , m_depth_pass_rs, sh_depth_pass_vs };
        // the used depth buffers have the format DXGI_FORMAT_D32_FLOAT
        m_depth_pass_pso.set_depth_format(DXGI_FORMAT_D32_FLOAT);
        // dx12u::pipeline_state_object create PSO objects in a lazy manner. The method commit ensures pso creation
        m_depth_pass_pso.commit();

        // color pass PSO
        m_color_pass_pso = dx12u::pipeline_state_object{ m_dev, ill , m_color_pass_rs, color_pass_vs, color_pass_ps };
        // the color pass doesn't update the depth buffer. It uses a previously built depth buffer
        m_color_pass_pso.set_depth_write(false);
        m_color_pass_pso.set_depth_func(D3D12_COMPARISON_FUNC_LESS_EQUAL);
        m_color_pass_pso.set_depth_format(DXGI_FORMAT_D32_FLOAT);
        // commit th PSO
        m_color_pass_pso.commit();
    }


    /////////////////////////////////////////////////////////////////
    // geometry pass
    /////////////////////////////////////////////////////////////////

    void draw_geometry(size_t frame_lid, size_t depth_pass_idx, bool color_pass)
    {
        // set common root parameters
        if (color_pass)
        {
            // in the color pass the shadow mask is bound as SRV
            m_cmd_list[frame_lid]->SetGraphicsRootDescriptorTable(1, m_srd_heap.get_gpu_handle(frame_lid * m_num_srd_per_frame + m_sh_mask_offset));
        }
        else
        {
            // bind the depth pass constant buffer
            m_cmd_list[frame_lid]->SetGraphicsRootDescriptorTable(0, m_srd_heap.get_gpu_handle(frame_lid * m_num_srd_per_frame + m_mesh_depth_pass_cb_offset + depth_pass_idx));
        }

        for (size_t m = 0; m < m_num_mesh; ++m)
        {
            // skip empty meshes
            if (m_mesh_vbo[m].vb.Get() == nullptr)
            {
                continue;
            }

            if (color_pass)
            {
                // bind plastic shader constant buffer
                // in the color pass each mesh may have a different material
                m_cmd_list[frame_lid]->SetGraphicsRootDescriptorTable(0, m_srd_heap.get_gpu_handle(frame_lid * m_num_srd_per_frame + m * 2));
            }

            // bind the vertex buffer
            m_cmd_list[frame_lid]->IASetVertexBuffers(0, 1, &m_mesh_vbo[m].vbv);
            m_cmd_list[frame_lid]->IASetIndexBuffer(&m_mesh_vbo[m].ibv);

            // draw
            m_cmd_list[frame_lid]->DrawIndexedInstanced(m_mesh_vbo[m].ibv.SizeInBytes / sizeof(uint32_t), 1, 0, 0, 0);
        }
    }


    /////////////////////////////////////////////////////////////////
    // depth pass
    /////////////////////////////////////////////////////////////////

    void render_depth_pass(size_t frame_lid)
    {
        // bind the depth pass PSO
        bind(m_cmd_list[frame_lid].Get(), m_depth_pass_pso);

        // repeat the depth pass for the view space and every point light cube face
        // depth_pass_idx zero will be used for the view space depth
        for (size_t depth_pass_idx = 0; depth_pass_idx < NUM_CUBE_FACE + 1; ++depth_pass_idx)
        {
            render_depth_pass(frame_lid, depth_pass_idx);
        }
    }

    void render_depth_pass(size_t frame_lid, size_t depth_pass_idx)
    {
        // depth_pass_idx is 0 for the view space depth pass, otherwise it's a shadow depth pass
        auto w = depth_pass_idx == 0 ? dx12u::get_window_width() : g_setting.shadow_res;
        auto h = depth_pass_idx == 0 ? dx12u::get_window_height() : g_setting.shadow_res;

        // set the viewport and scissor
        dx12u::set_viewport_scissor(m_cmd_list[frame_lid].Get(), w, h);

        // bind the depth render target
        // no color is bound
        m_cmd_list[frame_lid]->OMSetRenderTargets(0, nullptr, true, &m_dsv_heap.get_cpu_handle(frame_lid * m_num_dsv_per_frame + depth_pass_idx));

        // clear the depth buffer
        m_cmd_list[frame_lid]->ClearDepthStencilView(m_dsv_heap.get_cpu_handle(frame_lid * m_num_dsv_per_frame + depth_pass_idx), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);

        // draw the geometry
        draw_geometry(frame_lid, depth_pass_idx, false);
    }


    /////////////////////////////////////////////////////////////////
    // color pass
    /////////////////////////////////////////////////////////////////

    void render_color_pass(size_t frame_lid)
    {
        // set states
        dx12u::set_viewport_scissor(m_cmd_list[frame_lid].Get(), dx12u::get_window_width(), dx12u::get_window_height());
        bind(m_cmd_list[frame_lid].Get(), m_color_pass_pso);

        // bind back buffer
        m_cmd_list[frame_lid]->OMSetRenderTargets(1, &m_rtv_heap.get_cpu_handle(frame_lid), false, &m_dsv_heap.get_cpu_handle(frame_lid * m_num_dsv_per_frame + 0));

        // clear back buffer
        float const clear_color[] = { 0.f, 0.f, 0.f, 1.0f };
        m_cmd_list[frame_lid]->ClearRenderTargetView(m_rtv_heap.get_cpu_handle(frame_lid), clear_color, 0, nullptr);

        // draw the geometry
        draw_geometry(frame_lid, 0, true);
    }


    /////////////////////////////////////////////////////////////////
    // shadow filtering pass
    /////////////////////////////////////////////////////////////////

    void render_shadow_mask(size_t frame_lid)
    {
        // bind m_shadow_mask (check m_rtv_heap declaration for comments related to the layout of the heap)
        m_cmd_list[frame_lid]->OMSetRenderTargets(1, &m_rtv_heap.get_cpu_handle(m_num_buffered_frame + frame_lid), true, nullptr); 

        // clear m_shadow_mask render target
        float const clear_color[] = { 0.f, 0.f, 0.f, 1.0f };
        m_cmd_list[frame_lid]->ClearRenderTargetView(m_rtv_heap.get_cpu_handle(m_num_buffered_frame + frame_lid), clear_color, 0, nullptr);

        // set the shadow library instance. This allows shadow_fx to use the right constant buffer for this frame
        m_shadow_desc[frame_lid].m_InstanceID = static_cast<uint32_t>(frame_lid);

        // set the commad list that shadow_fx will use
        m_shadow_desc[frame_lid].m_CommandList = m_cmd_list[frame_lid].Get();

        // execute shadow_fx
        auto sh_err = AMD::ShadowFX_Render(m_shadow_desc[frame_lid]);
        process_shadow_fx_error(sh_err);
    }


    /////////////////////////////////////////////////////////////////
    // depth DSV to SRV barrier
    /////////////////////////////////////////////////////////////////

    void barrier_dsv_to_srv(size_t frame_lid)
    {
        // every frame starts with a depth pass
        // after the depth pass all depth resources will be in read only mode in the remaining of the frame
        // this function will be called after finishing the depth pass to transition depth resources to be in read mode

        auto depth_read_state = D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        auto depth_write_state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
        D3D12_RESOURCE_BARRIER const depth_write2read_barrier[2] =
        {
            CD3DX12_RESOURCE_BARRIER::Transition(m_view_space_db[frame_lid].Get(), depth_write_state, depth_read_state),
            CD3DX12_RESOURCE_BARRIER::Transition(m_light_space_db[frame_lid].Get(), depth_write_state, depth_read_state)
        };

        m_cmd_list[frame_lid]->ResourceBarrier(2, depth_write2read_barrier);
    }


    /////////////////////////////////////////////////////////////////
    // color pass barrier
    /////////////////////////////////////////////////////////////////

    void barrier_color_pass_start(size_t frame_lid)
    {
        // the color pass used m_shadow_mask and renders to the main color buffer

        // set the shadow mask as SRV
        // set the frame buffer as RTV for rendering
        D3D12_RESOURCE_BARRIER const color_start_barrier[] =
        {
            CD3DX12_RESOURCE_BARRIER::Transition(m_shadow_mask[frame_lid].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
            CD3DX12_RESOURCE_BARRIER::Transition(m_frame_buffer[frame_lid].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET)
        };

        m_cmd_list[frame_lid]->ResourceBarrier(std::extent<decltype(color_start_barrier)>::value, color_start_barrier);
    }


    /////////////////////////////////////////////////////////////////
    // end of frame barrier
    /////////////////////////////////////////////////////////////////

    void barrier_end_of_frame(size_t frame_lid)
    {
        // for the end of the frame the color buffer transitions to a presentable mode
        // also in the same batch make the shadow mask and all depth buffers writable for the next frame

        auto depth_read_state = D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        auto depth_write_state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
        D3D12_RESOURCE_BARRIER const end_barrier[] =
        {
            CD3DX12_RESOURCE_BARRIER::Transition(m_frame_buffer[frame_lid].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT),
            CD3DX12_RESOURCE_BARRIER::Transition(m_shadow_mask[frame_lid].Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET),
            CD3DX12_RESOURCE_BARRIER::Transition(m_view_space_db[frame_lid].Get(), depth_read_state, depth_write_state),
            CD3DX12_RESOURCE_BARRIER::Transition(m_light_space_db[frame_lid].Get(), depth_read_state, depth_write_state)
        };

        m_cmd_list[frame_lid]->ResourceBarrier(std::extent<decltype(end_barrier)>::value, end_barrier);
    }


    /////////////////////////////////////////////////////////////////
    // main rendering function
    /////////////////////////////////////////////////////////////////

    void render()
    {
        // there's no event handler for alt-tab while in fullscreen mode
        handle_alt_tab();

        // get the current frame id and the previous frame for which a wait is necessary
        auto frame_lid = m_swp_chain.get_current_buffer_idx();
        assert(frame_lid < m_num_buffered_frame);

        // compute the current frame fence and the previous frame fence for which a wait is necessary
        auto prev_frame_fence = m_frame_id + 1; // previous fence value. 1 is added to avoid zero fences
        auto curr_frame_fence = prev_frame_fence + m_num_buffered_frame; // current fence value is m_num_buffered_frame greater than the fence for which we need to wait

        // wait for previous fence
        wait_for_previous_fence(prev_frame_fence);

        // update timers
        float e = static_cast<float>(m_elapse.get());
        if (!m_pause)
        {
            // the elapsed time only increases if m_pause is not set
            m_elapsed_since_begin += e;
        }

        // update camera
        update_camera(e);

        // update constant buffers
        update_const_buffer(frame_lid, m_elapsed_since_begin);

        // reset the command allocator
        m_cmd_allocator[frame_lid].reset();

        // create a new command list
        auto r = m_cmd_list[frame_lid]->Reset(m_cmd_allocator[frame_lid].get_ptr(), nullptr);
        dx12u::throw_if_error(r);

        // D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST will be used in all the draws
        m_cmd_list[frame_lid]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // bind m_srd_heap. the heap will be used for all sample draws except the shadow_fx filtering pass
        dx12u::bind_descriptor_heap(m_cmd_list[frame_lid].Get(), m_srd_heap.get_com_ptr().Get());

        // view and light space depth passes
        render_depth_pass(frame_lid);

        // make all depth buffers readable to use them as SRV for shadow filtering and the color pass
        barrier_dsv_to_srv(frame_lid);

        // make the shadow mask that will hold the shadow filtering result
        render_shadow_mask(frame_lid);

        // bind m_srd_heap again as the shadow_fx library changes the heap
        dx12u::bind_descriptor_heap(m_cmd_list[frame_lid].Get(), m_srd_heap.get_com_ptr().Get());

        // set m_shadow_mask as SRV and the color buffer as RTV
        barrier_color_pass_start(frame_lid);

        // render color
        render_color_pass(frame_lid);

        // draw text
        draw_text(frame_lid);

        // end of frame barrier
        barrier_end_of_frame(frame_lid);

        // finish m_cmd_list recording
        r = m_cmd_list[frame_lid]->Close();
        dx12u::throw_if_error(r);

        // execute m_cmd_list
        m_queue.push(m_cmd_list[frame_lid]);

        // present
        m_swp_chain.present();

        // signal end of frame
        m_queue.set_fence(curr_frame_fence);
        // inc frame id
        ++m_frame_id;
    }


    /////////////////////////////////////////////////////////////////
    // wait for previous fence
    /////////////////////////////////////////////////////////////////

    void wait_for_previous_fence(size_t prev_frame_fence)
    {
        // no need to wait if we haven't rendered m_num_buffered_frame yet
        if (m_frame_id < m_num_buffered_frame)
        {
            return; // no previous frame rendered
        }

        // the method check_fence returns true if the fence is already reached
        if (m_queue.check_fence(prev_frame_fence))
        {
            return; // no need to wait as the fence is reach already
        }

        // wait for the fence
        m_queue.wait(prev_frame_fence);
    }


    /////////////////////////////////////////////////////////////////
    // wait for all fences
    /////////////////////////////////////////////////////////////////

    void wait_for_all_fences()
    {
        // the last rendered frame has the id = m_frame_id + m_num_buffered_frame
        // only a wait for the latest frame is needed as frames end in order
        m_queue.sync();
        auto last_frame_frence = m_frame_id + m_num_buffered_frame;
        wait_for_previous_fence(last_frame_frence);
    }


    /////////////////////////////////////////////////////////////////
    // update constant buffer
    /////////////////////////////////////////////////////////////////

    void update_const_buffer(size_t frame_lid, float time)
    {
        // world space light positions
        // the light position spins based on time
        float light_rot_angle = -time * 1.1f;
        tml::mat4 light_rot = tml::rotate(tml::mat4{}, light_rot_angle, tml::vec3(0.f, 1.f, 0.f));
        auto ws_lp = tml::vec3(light_rot * tml::vec4(m_light_pos, 1.f));

        // fov constants
        float const light_fov = 1.6f;
        float const view_fov = .8f;

        // light proj matrix
        tml::mat4 light_proj = gu::perspective_dx(light_fov, 1.f, 1.f, 39.f);

        // light view and view projection matrices
        tml::mat4 light_view[NUM_CUBE_FACE];
        tml::mat4 light_vp[NUM_CUBE_FACE];
        for (size_t i = 0; i < NUM_CUBE_FACE; ++i)
        {
            light_view[i] = gu::get_point_light_camera(ws_lp, i).get_view_matrix();
            light_vp[i] = light_proj * light_view[i];
        }

        // view matrix
        tml::mat4 view = m_camera.get_view_matrix();

        // projection matrix
        float aspect = static_cast<float>(dx12u::get_window_width()) / static_cast<float>(dx12u::get_window_height());
        tml::mat4 proj = gu::perspective_dx(view_fov, aspect, .2f, 100.f);

        // view projection
        tml::mat4 vp = proj * view;

        // if m_enable_debug is on the light point of view is set as camera
        // m_light_debug_id defines what cube face is used as camera
        if (m_enable_debug)
        {
            view = light_view[m_light_debug_id];
            proj = light_proj;
            vp = light_vp[m_light_debug_id];
        }

        // view space light position
        auto vs_lp = tml::vec3(view * tml::vec4(ws_lp, 1.f));

        // for each cube face and each mesh set the view space light position in plastic_ps_cb
        for (size_t i = 0; i < NUM_CUBE_FACE; ++i)
        {
            for (size_t m = 0; m < m_num_mesh; ++m)
            {
                m_mesh_plastic_ps_cb[frame_lid][m]->light_pos[i] = tml::vec4(vs_lp, 1.f);
            }
        }

        // update plastic_vs_cb const buffers
        m_mesh_plastic_vs_cb[frame_lid]->mv = view;
        m_mesh_plastic_vs_cb[frame_lid]->mvp = vp;

        // update depth_pass_cb const buffers
        m_mesh_depth_pass_cb[frame_lid][0]->mvp = vp;
        for (size_t i = 0; i < NUM_CUBE_FACE; ++i)
        {
            m_mesh_depth_pass_cb[frame_lid][i + 1]->mvp = light_vp[i];
        }

        // update shadow_fx view matrices
        // shadow_fx needs the camera information for the viewer and the light
        // glm is used in this sample to manage matrices
        // glm follows the conventional OpenGL matrix representation while shadow_fx follows D3D
        // glm matrices will be transposed before copying them to shadow_fx desc structure
        auto tvp = tml::transpose(vp);
        std::copy(&tvp[0][0], &tvp[0][0] + 16, m_shadow_desc[frame_lid].m_Viewer.m_ViewProjection.m);
        auto vp_inv = tml::inverse(vp);
        auto tvp_inv = tml::transpose(vp_inv);
        std::copy(&tvp_inv[0][0], &tvp_inv[0][0] + 16, m_shadow_desc[frame_lid].m_Viewer.m_ViewProjection_Inv.m);

        // update shadow_fx light matrices
        for (size_t i = 0; i < NUM_CUBE_FACE; ++i)
        {
            auto tlight_vp = tml::transpose(light_vp[i]);
            std::copy(&tlight_vp[0][0], &tlight_vp[0][0] + 16, m_shadow_desc[frame_lid].m_Light[i].m_ViewProjection.m);
            vp_inv = tml::inverse(light_vp[i]);
            tvp_inv = tml::transpose(vp_inv);
            std::copy(&tvp_inv[0][0], &tvp_inv[0][0] + 16, m_shadow_desc[frame_lid].m_Light[i].m_ViewProjection_Inv.m);
        }
    }


    /////////////////////////////////////////////////////////////////
    // update swap chain
    /////////////////////////////////////////////////////////////////

    void update_swap_chain()
    {
        // update_swap_chain is normally called following a WM_SIZE event
        // ignore WM_SIZE if the swap chain is busy switching to modes
        if (m_swp_chain.is_valid() && m_swp_chain.is_busy())
        {
            return;
        }

        // zero sized swap chain is invalid
        if (!dx12u::get_window_height() || !dx12u::get_window_width())
        {
            return;
        }

        // make sure all swap chain dependent work is done
        wait_for_all_fences();

        // delete previous swap chain and dependent objects
        for (size_t i = 0; i < m_num_buffered_frame; ++i)
        {
            m_frame_buffer[i] = nullptr;
        }

        // for simplicity we delete and recreate the swap chain
        m_swp_chain = dx12u::swap_chain{};

        // create a new swap chain
        m_swp_chain = dx12u::swap_chain{ m_queue, dx12u::get_hwnd(), dx12u::get_window_width(), dx12u::get_window_height(), m_num_buffered_frame };

        // create a RTV for each color buffer in the chain
        for (size_t i = 0; i < m_num_buffered_frame; ++i)
        {
            m_frame_buffer[i] = m_swp_chain.get_buffer(i);
            m_dev->CreateRenderTargetView(m_frame_buffer[i].Get(), nullptr, m_rtv_heap.get_cpu_handle(i));
        }

        // create or re-create resolution dependent resources
        create_view_dependent_resources();
    }


    /////////////////////////////////////////////////////////////////
    // function to call before destroying the swap chain
    /////////////////////////////////////////////////////////////////

    void on_destroy_swap_chain()
    {
        // on destroy make sure the swap chain is not in fullscreen mode

        // wait for all work to finish
        wait_for_all_fences();

        // delete swap chain dependent objects
        for (size_t i = 0; i < m_num_buffered_frame; ++i)
        {
            m_frame_buffer[i] = nullptr;
        }

        // set the swap chain in windowed mode
        if (m_swp_chain.is_valid() && m_swp_chain.is_fullscreen())
        {
            m_swp_chain.set_fullscreen(false);
        }
    }


    /////////////////////////////////////////////////////////////////
    // adjust swap chain resolution
    /////////////////////////////////////////////////////////////////

    void adjust_resolution()
    {
        // before switching to fullscreen we explicitely set the resolution to one supported by the monitor
        auto mode = m_swp_chain.query_closest_mode(g_setting.width, g_setting.height);
        g_setting.width = mode.x;
        g_setting.height = mode.y;

        // set the new size if it changed
        dx12u::resize_window(g_setting.width, g_setting.height);
        update_swap_chain();
    }


    /////////////////////////////////////////////////////////////////
    // toggle between fullscreen and windowed
    /////////////////////////////////////////////////////////////////

    void switch_window_mode()
    {
        bool old_fullscreen_sate = m_swp_chain.is_fullscreen();
        bool new_fullscreen_sate = !old_fullscreen_sate;
        if (new_fullscreen_sate == true)
        {
            // adjust resolution before going into fullscreen mode
            adjust_resolution();
        }

        // make sure all swap chain dependent work is done
        wait_for_all_fences();

        // delete swap chain dependent objects
        for (size_t i = 0; i < m_num_buffered_frame; ++i)
        {
            m_frame_buffer[i] = nullptr;
        }

        // change mode
        m_swp_chain.set_fullscreen(new_fullscreen_sate);

        // update resolution dependent maps
        for (size_t i = 0; i < m_num_buffered_frame; ++i)
        {
            m_frame_buffer[i] = m_swp_chain.get_buffer(i);
            m_dev->CreateRenderTargetView(m_frame_buffer[i].Get(), nullptr, m_rtv_heap.get_cpu_handle(i));
        }
        create_view_dependent_resources();

        // update m_is_fullscreen tracker
        m_is_fullscreen = m_swp_chain.is_fullscreen();
    }


    /////////////////////////////////////////////////////////////////
    // update swap chain in response to alt-tab
    /////////////////////////////////////////////////////////////////

    void handle_alt_tab()
    {
        // there's no event handler for alt-tab while in fullscreen mode
        // we detect alt-tab by checking m_is_fullscreen againt the actual swap chain mode
        if (m_is_fullscreen == m_swp_chain.is_fullscreen())
        {
            return;
        }
        // else mismatch: handle fullscreen/windowed switch caused by alt-tab

        // make sure all swap chain dependent work is done
        wait_for_all_fences();

        // delete swap chain dependent objects
        for (size_t i = 0; i < m_num_buffered_frame; ++i)
        {
            m_frame_buffer[i] = nullptr;
        }

        // resize swap chain
        m_swp_chain.set_fullscreen(m_swp_chain.is_fullscreen());

        // update resolution dependent maps
        for (size_t i = 0; i < m_num_buffered_frame; ++i)
        {
            m_frame_buffer[i] = m_swp_chain.get_buffer(i);
            m_dev->CreateRenderTargetView(m_frame_buffer[i].Get(), nullptr, m_rtv_heap.get_cpu_handle(i));
        }
        create_view_dependent_resources();

        // update m_is_fullscreen tracker
        m_is_fullscreen = m_swp_chain.is_fullscreen();
    }

    
    /////////////////////////////////////////////////////////////////
    // draw a simple UI text
    /////////////////////////////////////////////////////////////////

    void draw_text(size_t frame_lid)
    {
        if (!m_enable_text)
        {
            // text disabled
            return;
        }

        // set the mode text depending on whether we're in fullscreen or windowed mode
        std::string const switch_mode_text = m_is_fullscreen ? "(F5) windowed" : "(F5) fullscreen";

        // set one character width and height
        auto charw = m_ui_text.get_char_w(dx12u::get_window_width()) * .8f;
        auto charh = m_ui_text.get_char_h(dx12u::get_window_height()) * .8f;

        // set spacing between lines
        auto spacing = charh * 2.f;

        // set the text starting position
        auto txt_start = tml::vec2(1.f - charw * 38.f, 1.f - spacing);

        // clear the text container
        m_ui_text.clear(frame_lid);

        // print fps
        m_ui_text.add_text(frame_lid, txt_start.x, txt_start.y, charw, charh, std::string{ "fps:" } +std::to_string(m_fps));
        txt_start.y -= spacing;

        // print show text, window mode and pause options
        m_ui_text.add_text(frame_lid, txt_start.x, txt_start.y, charw, charh, "(F1) show text");
        txt_start.y -= spacing;

        m_ui_text.add_text(frame_lid, txt_start.x, txt_start.y, charw, charh, switch_mode_text);
        txt_start.y -= spacing;

        m_ui_text.add_text(frame_lid, txt_start.x, txt_start.y, charw, charh, "(P) pause");
        txt_start.y -= spacing;

        // print shadow parameters
        m_ui_text.add_text(frame_lid, txt_start.x, txt_start.y, charw, charh, "shadow");
        txt_start.y -= spacing;

        m_ui_text.add_text(frame_lid, txt_start.x, txt_start.y, charw, charh, std::string{ "(1) dimension:" } +std::to_string(g_setting.shadow_res));
        txt_start.y -= spacing;

        m_ui_text.add_text(frame_lid, txt_start.x, txt_start.y, charw, charh, std::string{ "(2) filter size:" } +std::to_string(g_setting.filtersz));
        txt_start.y -= spacing;

        m_ui_text.add_text(frame_lid, txt_start.x, txt_start.y, charw, charh, std::string{ "(3) " } +g_setting.fetch);
        txt_start.y -= spacing;

        m_ui_text.add_text(frame_lid, txt_start.x, txt_start.y, charw, charh, std::string{ "(4) " } +g_setting.filter);
        txt_start.y -= spacing;

        m_ui_text.add_text(frame_lid, txt_start.x, txt_start.y, charw, charh, std::string{ "(5) " } +g_setting.tap);
        txt_start.y -= spacing;

        std::ostringstream oss;
        oss << "(+/-) bias:" << std::setprecision(5) << g_setting.depth_bias;
        m_ui_text.add_text(frame_lid, txt_start.x, txt_start.y, charw, charh, oss.str().c_str());

        // draw the text
        m_ui_text.draw(frame_lid, m_cmd_list[frame_lid].Get());

        // update the fps
        ++m_frame_counter;
        if (m_second_counter.get_ms() >= 1000)
        {
            m_fps = std::min<size_t>(m_frame_counter, 999);
            m_frame_counter = 0;
            m_second_counter.reset();
        }
    }


    /////////////////////////////////////////////////////////////////
    // update the camera in responce to the mouse
    /////////////////////////////////////////////////////////////////

    void update_camera(float x, float y)
    {
        m_camera.update_yaw010(x);
        m_camera.update_pitch(y);
    }


    /////////////////////////////////////////////////////////////////
    // update the camera in responce to the keyboard
    /////////////////////////////////////////////////////////////////

    void update_camera(float e)
    {
        float const rot_step = e * 1.f;
        float const move_step = e * 5.f;
        if (dx12u::is_key_pressed('W'))
        {
            m_camera.move(move_step);
        }
        if (dx12u::is_key_pressed('S'))
        {
            m_camera.move(-move_step);
        }
        if (dx12u::is_key_pressed('D'))
        {
            m_camera.strafe(move_step);
        }
        if (dx12u::is_key_pressed('A'))
        {
            m_camera.strafe(-move_step);
        }
        if (dx12u::is_key_pressed(VK_RIGHT))
        {
            m_camera.update_yaw010(rot_step);
        }
        if (dx12u::is_key_pressed(VK_LEFT))
        {
            m_camera.update_yaw010(-rot_step);
        }
        if (dx12u::is_key_pressed(VK_UP))
        {
            m_camera.update_pitch(rot_step);
        }
        if (dx12u::is_key_pressed(VK_DOWN))
        {
            m_camera.update_pitch(-rot_step);
        }
    }


    /////////////////////////////////////////////////////////////////
    // process keyboard
    /////////////////////////////////////////////////////////////////

    void process_keyboard(std::uint8_t k)
    {
        if (m_frame_id == 0)
        {
            // make sure any initialization is done before changing shadow parameters
            return;
        }

        // show/hide text
        if (k == VK_F1)
        {
            m_enable_text = !m_enable_text;
        }

        // pause/unpause
        else if (k == 'P')
        {
            m_pause = !m_pause;
        }

        // debug light camera
        else if (k == 'L')
        {
            if (!m_enable_debug)
            {
                m_enable_debug = true;
                m_light_debug_id = 0;
            }
            else
            {
                ++m_light_debug_id;
                if (m_light_debug_id >= NUM_CUBE_FACE)
                {
                    m_enable_debug = false;
                }
            }
        }

        // change shadow resolutin
        else if (k == '1' || k == VK_NUMPAD1)
        {
            // rotate between 1024 -> 4096
            g_setting.shadow_res = g_setting.shadow_res * 2;
            if (g_setting.shadow_res > 4096)
            {
                g_setting.shadow_res = 1024;
            }

            // update shadow_fx desc
            for (size_t frame_lid = 0; frame_lid < m_num_buffered_frame; ++frame_lid)
            {
                for (size_t i = 0; i < NUM_CUBE_FACE; ++i)
                {
                    m_shadow_desc[frame_lid].m_ShadowSize[i].x = static_cast<float>(g_setting.shadow_res);
                    m_shadow_desc[frame_lid].m_ShadowSize[i].y = static_cast<float>(g_setting.shadow_res);
                }
            }

            // re-create shadow maps
            wait_for_all_fences();
            create_shadow_map_resources();
        }

        // change shadow filter size
        else if (k == '2' || k == VK_NUMPAD2)
        {
            g_setting.filtersz += 2;
            if (g_setting.filtersz > 15)
            {
                g_setting.filtersz = 7;
            }
            for (size_t frame_lid = 0; frame_lid < m_num_buffered_frame; ++frame_lid)
            {
                m_shadow_desc[frame_lid].m_FilterSize = static_cast<AMD::SHADOWFX_FILTER_SIZE>(g_setting.filtersz);
            }
        }

        // change shadow texture fetch technique
        else if (k == '3' || k == VK_NUMPAD3)
        {
            g_setting.fetch = g_setting.fetch == "gather" ? "pcf" : "gather";
            for (size_t frame_lid = 0; frame_lid < m_num_buffered_frame; ++frame_lid)
            {
                m_shadow_desc[frame_lid].m_TextureFetch = g_setting.fetch == "gather" ? AMD::SHADOWFX_TEXTURE_FETCH_GATHER4 : AMD::SHADOWFX_TEXTURE_FETCH_PCF;
            }
        }

        // change shadow filtering mode
        else if (k == '4' || k == VK_NUMPAD4)
        {
            g_setting.filter = g_setting.filter == "uniform" ? "contact" : "uniform";
            for (size_t frame_lid = 0; frame_lid < m_num_buffered_frame; ++frame_lid)
            {
                m_shadow_desc[frame_lid].m_Filtering = g_setting.filter == "uniform" ? AMD::SHADOWFX_FILTERING_UNIFORM : AMD::SHADOWFX_FILTERING_CONTACT;
            }
        }

        // change shadow sampling mode
        else if (k == '5' || k == VK_NUMPAD5)
        {
            g_setting.tap = g_setting.tap == "fixed" ? "poisson" : "fixed";
            for (size_t frame_lid = 0; frame_lid < m_num_buffered_frame; ++frame_lid)
            {
                m_shadow_desc[frame_lid].m_TapType = g_setting.tap == "fixed" ? AMD::SHADOWFX_TAP_TYPE_FIXED : AMD::SHADOWFX_TAP_TYPE_POISSON;
            }
        }

        // change z bias
        else if (k == VK_ADD)
        {
            g_setting.depth_bias += 0.0001f;
        }
        else if (k == VK_SUBTRACT)
        {
            g_setting.depth_bias -= 0.0001f;
        }

        // toggle fullscreen/windowed
        else if (k == VK_F5)
        {
            switch_window_mode();
        }

        // update depth bias in case it changed
        for (size_t frame_lid = 0; frame_lid < m_num_buffered_frame; ++frame_lid)
        {
            std::fill(std::begin(m_shadow_desc[frame_lid].m_DepthTestOffset), std::end(m_shadow_desc[frame_lid].m_DepthTestOffset), g_setting.depth_bias);
        }
    }


    /////////////////////////////////////////////////////////////////
    // release shadow_fx
    /////////////////////////////////////////////////////////////////

    ~shadow_fx_sample()
    {
        wait_for_all_fences();

        auto sh_err = AMD::ShadowFX_Release(m_shadow_desc[0]);
        process_shadow_fx_error(sh_err);
    }
    

    /////////////////////////////////////////////////////////////////
    // handle shadow_fx errors
    /////////////////////////////////////////////////////////////////

    void process_shadow_fx_error(AMD::SHADOWFX_RETURN_CODE code)
    {
        if (code != AMD::SHADOWFX_RETURN_CODE_SUCCESS)
        {
            throw std::runtime_error{ "shadow_fx error" };
        }
    }
};


/////////////////////////////////////////////////////////////////
// global variables
/////////////////////////////////////////////////////////////////

namespace
{
    // shadow_fx_sample object
    std::unique_ptr<shadow_fx_sample> g_shadow_fx_sample{ nullptr };

    // The user can move the camera with the mouse when the left mouse button is held
    // g_is_lmouse_down keeps track of the left mouse button state
    bool g_is_lmouse_down = false;

}


/////////////////////////////////////////////////////////////////
// application initialization
/////////////////////////////////////////////////////////////////

void init()
{
    // create the shadow_fx_sample object
    g_shadow_fx_sample.reset(new shadow_fx_sample);

    // set fullscreen if the command line set the mode
    if (!g_setting.is_windowed)
    {
        g_shadow_fx_sample->switch_window_mode();
    }
}


/////////////////////////////////////////////////////////////////
// application rendering function
/////////////////////////////////////////////////////////////////

void render()
{
    g_shadow_fx_sample->render();
}


/////////////////////////////////////////////////////////////////
// application swap chain change handler
/////////////////////////////////////////////////////////////////

void update_swap_chain()
{
    g_shadow_fx_sample->update_swap_chain();
}


/////////////////////////////////////////////////////////////////
// application cleanup function
/////////////////////////////////////////////////////////////////

void release()
{
    g_shadow_fx_sample->on_destroy_swap_chain();
    g_shadow_fx_sample = nullptr;
}


/////////////////////////////////////////////////////////////////
// application keyboard handler
/////////////////////////////////////////////////////////////////

void process_keyboard(std::uint8_t k)
{
    g_shadow_fx_sample->process_keyboard(k);
}


/////////////////////////////////////////////////////////////////
// application mouse button handler
/////////////////////////////////////////////////////////////////

void process_mouse(dx12u::mouse_button left, dx12u::mouse_button right, std::size_t x, std::size_t y)
{
    if (left == dx12u::mouse_button::down)
    {
        g_is_lmouse_down = true;
        dx12u::set_mouse_capture(true);
    }
    else if (left == dx12u::mouse_button::up)
    {
        g_is_lmouse_down = false;
        dx12u::set_mouse_capture(false);
    }
}


/////////////////////////////////////////////////////////////////
// application mouse position handler
/////////////////////////////////////////////////////////////////

void process_mouse_motion(float x, float y)
{
    if (g_is_lmouse_down)
    {
        g_shadow_fx_sample->update_camera(x, y);
    }
}


/////////////////////////////////////////////////////////////////
// command line parser
/////////////////////////////////////////////////////////////////

void parse_cmd_line(int argc, char** argv)
{
    std::string const help_msg = "\
-windowed (binary): 1 windowed, 0 fullscreen \n\
-width (number): window width \n\
-height (number): window height \n\
\n\
-shadowres (number): shadow map resolution \n\
-fetch (string): gather or pcf \n\
-filtersz (number): filter size 7 9 11 13 or 15\n\
-filter (string): uniform or contact\n\
-tap (string): fixed or poisson\n\
";
    bool help = true;
    gu::cmd_line cmline{ argc, argv };
    cmline.get_bool("windowed", g_setting.is_windowed);
    cmline.get_size_t("width", g_setting.width);
    cmline.get_size_t("height", g_setting.height);
    cmline.get_size_t("shadowres", g_setting.shadow_res);
    cmline.get_size_t("filtersz", g_setting.filtersz);
    cmline.get_string("fetch", g_setting.fetch);
    cmline.get_string("filter", g_setting.filter);
    cmline.get_string("tap", g_setting.tap);

    if (help)
    {
        std::cout << "command line usage:\n" << help_msg << std::endl;
    }

    if (g_setting.fetch != "gather" && g_setting.fetch != "pcf")
    {
        throw std::runtime_error{ "invalid command line: fetch must be gather or pcf" };
    }

    if (g_setting.filter != "uniform" && g_setting.filter != "contact")
    {
        throw std::runtime_error{ "invalid command line: filter must be uniform or contact" };
    }

    if (g_setting.tap != "fixed" && g_setting.tap != "poisson")
    {
        throw std::runtime_error{ "invalid command line: tap must be fixed or poisson" };
    }

    if (g_setting.tap != "fixed" && g_setting.tap != "poisson")
    {
        throw std::runtime_error{ "invalid command line: tap must be fixed or poisson" };
    }

    switch (static_cast<AMD::SHADOWFX_FILTER_SIZE>(g_setting.filtersz))
    {
    case AMD::SHADOWFX_FILTER_SIZE_7:
    case AMD::SHADOWFX_FILTER_SIZE_11:
    case AMD::SHADOWFX_FILTER_SIZE_13:
    case AMD::SHADOWFX_FILTER_SIZE_15:
        break;
    default: throw std::runtime_error{ "invalid command line: filtersz is invalid" };
    }
}


/////////////////////////////////////////////////////////////////
// main
/////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
    try
    {
        parse_cmd_line(argc, argv);

        dx12u::mainloop("shadow_fx_sample", g_setting.width, g_setting.height, render, init, release, update_swap_chain, process_keyboard, process_mouse, process_mouse_motion);
    }
    catch (std::exception const& e)
    {
        std::cerr << "error: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "unknown exception!" << std::endl;
    }
}

