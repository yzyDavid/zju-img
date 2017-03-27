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

    auto bmp = bitmap::from_file(filename);
    std::string new_name{std::string{filename} + "_gray.bmp"};
    std::string new_name2{std::string{filename} + "_lightened.bmp"};
    //assert(bmp->rgbquad_count() == 0);
    bmp->print_header(std::cout);
/*    for (unsigned int i = 0; i < 100; ++i)
    {
        bmp->position(100, i).blue = 255;
        bmp->position(101, i).green = 255;
        bmp->position(102, i).red = 255;
    }*/
    auto yuv = bmp->to_yuv();
    auto yuv2 = bmp->to_yuv();
    yuv->for_each([](yuv_image::yuv_pixel &p)
                  {
                      p.u = p.v = 0;
                  });
    yuv2->for_each([](yuv_image::yuv_pixel &p)
                   {
                       p.y *= 1.2;
                   });
    auto back = bitmap::from_yuv(yuv, bmp);
    back->write_to_file(new_name.c_str());
    back = bitmap::from_yuv(yuv2, bmp);
    back->write_to_file(new_name2.c_str());

    return 0;
}
