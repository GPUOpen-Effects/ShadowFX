
/**
* @file      texture.hpp
* @brief     texture utilities
* @copyright 2015 Advanced Micro Devices Inc
*/

#ifndef GU_TEXTURE_HPP
#define GU_TEXTURE_HPP

#include <cstddef>

namespace gu
{

    /**
    * @brief texture view for one mip level
    */
    struct mip_lvl_texture_view
    {
        std::size_t width = 0;  //!< texture width
        std::size_t height = 0;  //!< texture height
        std::size_t depth = 1;  //!< texture depth

        std::size_t pitch = 0;  //!< texture pitch: row size in bytes
        std::size_t slice_sz = 0;  //!< texture slice size: slice size in bytes
        void* data = nullptr;  //!< data
    };

    /**
    * @brief texture view
    */
    struct texture_view
    {
        std::size_t bpp = 0;  //!< bytes per pixel
        std::size_t num_lvl = 0;  //!< number of mip levels
        mip_lvl_texture_view* mip; //!< mip levels data
    };

} // namespace 

#endif // GU_TEXTURE_HPP
