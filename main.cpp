/*
 * written by yzy.
 * homework for image process course.
 * 2017.3.16
 */

#include <iostream>
#include <memory>
#include "bitmap.h"

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

    auto bmp = bitmap::from_file(filename);
    ;

    return 0;
}
