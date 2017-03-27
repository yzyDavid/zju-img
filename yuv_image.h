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

    template<typename T = uint8_t>
    inline __attribute__((always_inline)) T clamp(T v, T lb = 0, T ub = 255)
    {
        if (v < lb) return lb;
        if (v > ub) return ub;
        return v;
    }

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
            double y;
            double u;
            double v;
        }__attribute__((aligned(8)));

        static_assert(sizeof(yuv_pixel) == 24);

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

        yuv_pixel &operator[](size_t index)
        {
            return data[index];
        }

        yuv_pixel &at(size_t index)
        {
            return (*this)[index];
        }

        void for_each(std::function<void(yuv_pixel &)> op);

    private:
        yuv_meta meta;
        yuv_pixel *data;

        friend class wheel::bitmap;
    };
}

#endif //HW1_YUV_IMAGE_H
