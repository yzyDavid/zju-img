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
}

#endif //HW1_KERNEL_H
