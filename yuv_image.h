//
// Created by yzy on 3/17/17.
//

#ifndef HW1_YUV_IMAGE_H
#define HW1_YUV_IMAGE_H

#include <cstdint>

namespace wheel
{
    class yuv_image
    {
        struct yuv_meta //memory struct, not for a file type.
        {
            uint32_t height;
            uint32_t width;
        };

        struct yuv_pixel
        {
            uint8_t y : 8;
            uint8_t u : 8;
            uint8_t v : 8;
        }__attribute__((aligned(4)));

        static_assert(sizeof(yuv_pixel) == 4);
    };
}


#endif //HW1_YUV_IMAGE_H
