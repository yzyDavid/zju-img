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
    };
}


#endif //HW1_YUV_IMAGE_H
