
/**
* @file      buffer.ipp
* @brief     dx12 buffer helpers
* @copyright 2015 Advanced Micro Devices Inc
*/

#ifndef DX12UTIL_BUFFER_IPP
#define DX12UTIL_BUFFER_IPP

#include <cmd_mgr.hpp>
#include <memory.hpp>
#include <utility>

namespace dx12u { namespace detail
{
    inline std::pair<dx12u::resource, dx12u::resource> make_buffer(dx12u::device const& dvc, dx12u::gfx_cmd_list& cl,
        void const* data, std::size_t sz, dx12u::heap const& h, std::size_t heap_offset, D3D12_RESOURCE_STATES init_state)
    {
        if (!sz)
        {
            throw dx12u::error{ "empty buffer" };
        }
        
        auto buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(sz);

        dx12u::resource rsrc{ nullptr };
        if (h.Get() == nullptr)
        {
            auto default_heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
            auto r = dvc->CreateCommittedResource(&default_heap, D3D12_HEAP_FLAG_NONE,
                &buffer_desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&rsrc));
            dx12u::throw_if_error(r);
        }
        else
        {
            auto r = dvc->CreatePlacedResource(h.Get(), heap_offset,
                &buffer_desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&rsrc));
            dx12u::throw_if_error(r);
        }

        auto upload_heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        dx12u::resource rsrc_sys{ nullptr };
        auto r = dvc->CreateCommittedResource(&upload_heap, D3D12_HEAP_FLAG_NONE,
            &buffer_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&rsrc_sys));
        dx12u::throw_if_error(r);

        D3D12_SUBRESOURCE_DATA rsrc_desc = {};
        rsrc_desc.pData = data;
        rsrc_desc.RowPitch = sz;
        rsrc_desc.SlicePitch = sz;
        UpdateSubresources<1>(cl.Get(), rsrc.Get(), rsrc_sys.Get(), 0, 0, 1, &rsrc_desc);
        auto transition = CD3DX12_RESOURCE_BARRIER::Transition(rsrc.Get(), D3D12_RESOURCE_STATE_COPY_DEST, init_state);
        cl->ResourceBarrier(1, &transition);

        return
        {
            rsrc,
            rsrc_sys
        };
    }
}} // namespace

#endif // DX12UTIL_BUFFER_IPP
