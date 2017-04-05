//
// Created by yzy on 4/5/17.
//

#ifndef HW1_POINT_H
#define HW1_POINT_H

#include <cstdint>
#include <type_traits>

namespace wheel
{
    struct point
    {
    public:
        uint32_t x, y;
    }__attribute__((packed));

    static_assert(std::is_pod<point>::value);
}


#endif //HW1_POINT_H
