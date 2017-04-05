/*
 * written by yzy.
 * homework for image process course.
 * 2017.3.16
 */

#include <functional>
#include <string>
#include <iomanip>
#include <iostream>
#include <memory>
#include <cassert>

#include "bitmap.h"
#include "yuv_image.h"
#include "kernel.h"

void hw2_entry(const char *filename);

int main(int argc, char *argv[])
{
    using namespace wheel;

/*    std::clog << sizeof(bitmap::bitmap_file_header_type) << '\t';
    std::clog << sizeof(bitmap::bitmap_info_header_type);*/

    char *filename;
    switch (argc)
    {
        case 1:
            filename = new char[256];
            std::cin >> filename;
            break;
        case 2:
            filename = argv[1];
            break;
        default:
            return 1;
    }

    std::cout.setf(std::ios::hex | std::ios::showbase);
    std::cout << std::setbase(16);

    hw2_entry(filename);

    return 0;
}

void hw2_entry(const char *filename)
{
    using namespace std;
    using namespace wheel;

    auto bmp = bitmap::from_file(filename);

    auto ker = kernel<bitmap::rgb_pixel, 3, 3>{};
    ker.content[1][1] = bitmap::rgb_pixel{255, 255, 255};
    ker.content[1][0] = bitmap::rgb_pixel{255, 255, 255};
    ker.content[1][2] = bitmap::rgb_pixel{255, 255, 255};
    ker.content[0][1] = bitmap::rgb_pixel{255, 255, 255};
    ker.content[2][1] = bitmap::rgb_pixel{255, 255, 255};
    ker.center = {1, 1};
}
