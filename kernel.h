//
// Created by yzy on 4/5/17.
//

#ifndef HW1_KERNEL_H
#define HW1_KERNEL_H

#include <cstdint>
#include <type_traits>
#include <array>

#include "point.h"


namespace wheel
{
    template<typename TElement, uint8_t height, uint8_t width>
    struct kernel final
    {
    public:
        point center;
        std::array<std::array<TElement, width>, height> content;

        kernel(TElement init)
        {
            center = {(uint32_t) (width / 2), (uint32_t) (height / 2)};
            for (auto &i : content)
            {
                for (auto &j : i)
                {
                    j = init;
                }
            }
        }

        kernel()
                : kernel(TElement{})
        {}
    };

    struct float64_pixel
    {
        double blue;
        double green;
        double red;
    }__attribute((packed));

    static_assert(sizeof(float64_pixel) == 24);
    static_assert(std::is_pod<float64_pixel>::value);

    struct int_pixel
    {
        int blue;
        int green;
        int red;
    }__attribute((packed));

    static_assert(sizeof(int_pixel) == sizeof(int) * 3);
    static_assert(std::is_pod<int_pixel>::value);
}

#endif //HW1_KERNEL_H
