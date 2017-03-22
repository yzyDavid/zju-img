//
// Created by yzy on 3/17/17.
//

#include "yuv_image.h"

namespace wheel
{
    void yuv_image::for_each(std::function<void(yuv_pixel &)> op)
    {
        for (size_t i = 0; i < meta.height * meta.width; ++i)
        {
            op(data[i]);
        }
    }
}
