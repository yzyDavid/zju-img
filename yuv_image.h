//
// Created by yzy on 3/17/17.
//

#ifndef HW1_YUV_IMAGE_H
#define HW1_YUV_IMAGE_H

#include <cstdint>
#include <functional>

namespace wheel
{
    class bitmap;

    class yuv_image final
    {
    public:
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

        yuv_image() = delete;

        yuv_image(uint32_t height, uint32_t width)
                : data(new yuv_pixel[height * width])
        {
            meta.height = height;
            meta.width = width;
        }

        yuv_image(const yuv_image &) = delete;
        yuv_image(yuv_image &&) = delete;

        ~yuv_image()
        {
            delete[] data;
        }

        void for_each(std::function<void(yuv_pixel &)> op);

    private:
        yuv_meta meta;
        yuv_pixel *data;

        friend class wheel::bitmap;
    };
}

#endif //HW1_YUV_IMAGE_H
