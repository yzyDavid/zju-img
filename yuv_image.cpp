//
// Created by yzy on 3/17/17.
//

#include <memory>
#include <cstring>
#include <iostream>
#include <cmath>
#include <cassert>
#include <algorithm>

#include "yuv_image.h"

namespace wheel
{
    yuv_image::yuv_image(const yuv_image &that)
            : yuv_image(that.meta.height, that.meta.width)
    {
        std::memcpy(data, that.data, sizeof(yuv_pixel) * meta.height * meta.width);
    }

    void yuv_image::for_each(std::function<void(yuv_pixel &)> op)
    {
        for (size_t i = 0; i < meta.height * meta.width; ++i)
        {
            op(data[i]);
        }
    }

    std::shared_ptr<yuv_image> yuv_image::logarithmic() const
    {
        auto re = std::make_shared<yuv_image>(*this);
        double max_y = 0;
        re->for_each([&max_y](auto &px)
                     {
                         max_y = std::max(px.y, max_y);
                     });

        auto ly = std::log(max_y + 1);
        re->for_each([ly](auto &px)
                     {
                         px.y = 255 * std::log(px.y + 1) / ly;
                     });

        return re;
    }

    std::shared_ptr<yuv_image> yuv_image::histogram_equalize(unsigned int levels) const
    {
        auto re = std::make_shared<yuv_image>(*this);
        auto sum_pxs = re->meta.width * re->meta.height;
        auto *nk = new uint32_t[levels];
        memset(nk, 0, sizeof(uint32_t) * levels);
        double interval = 255.0 / levels;
        re->for_each([levels, nk, interval](const auto &px)
                     {
                         for (auto i = 0; i < levels; ++i)
                         {
                             if (px.y > interval * i)
                             {
                                 nk[i]++;
                                 break;
                             }
                         }
                     });

        int pxs = 0;
        for (int i = 0; i < levels; ++i)
        {
            pxs += nk[i];
        }
        assert(pxs == sum_pxs);

        double *sk = new double[levels];
        double sksum{0};
        for (auto i = 0; i < levels; ++i)
        {
            sk[i] = sksum += (double) nk[i] / sum_pxs;
        }

        int *lsk = new int[levels];
        for (int i = 0; i < levels; ++i)
        {
            for (int j = 0; j < levels; ++j)
            {
                if (sk[i] > j * interval)
                {
                    lsk[i] = j;
                }
            }
        }

        re->for_each([levels, lsk, interval](auto &px)
                     {
                         for (int i = 0; i < levels; ++i)
                         {
                             if (px.y > i * interval)
                             {
                                 px.y += interval * (lsk[i] - i);
                             }
                         }
                     });

        delete[] lsk;
        delete[] sk;
        delete[] nk;
        return re;
    }
}

