
#ifndef GU_MAKE_IB_HPP
#define GU_MAKE_IB_HPP

#include <mesh/gu_mesh.hpp>
#include <cassert>

inline gu::index_buffer make_ib(std::size_t num_circle, std::size_t num_pt)
{
    assert(num_circle > 0 && num_pt > 0);

    gu::index_buffer ib((num_circle - 1) * (num_pt - 1) * 6);
    ib.resize(0);

    for (std::size_t c = 0; c < num_circle - 1; ++c)
    {
        std::size_t c0_start = c * num_pt;
        std::size_t c1_start = c0_start + num_pt;
        for (std::size_t p = 0; p < num_pt - 1; ++p)
        {
            // quad
            auto pt0 = static_cast<std::uint32_t>(c0_start + p + 0);
            auto pt1 = static_cast<std::uint32_t>(c0_start + p + 1);
            auto pt2 = static_cast<std::uint32_t>(c1_start + p + 1);
            auto pt3 = static_cast<std::uint32_t>(c1_start + p + 0);
            // 2 x tri
            ib.push_back(pt0);
            ib.push_back(pt1);
            ib.push_back(pt3);
            ib.push_back(pt3);
            ib.push_back(pt1);
            ib.push_back(pt2);
        }
    }

    assert(ib.size() == (num_circle - 1) * (num_pt - 1) * 6);

    return ib;
}


#endif // GU_MAKE_IB_HPP
