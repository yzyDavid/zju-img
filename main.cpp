/*
 * written by yzy.
 * homework for image process course.
 * 2017.3.16
 */

#include <string>
#include <iomanip>
#include <iostream>
#include <memory>
#include <cassert>

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

    std::cout.setf(std::ios::hex | std::ios::showbase);
    std::cout << std::setbase(16);

    auto bmp = bitmap::from_file(filename);
    std::string new_name{std::string{filename} + ".new.bmp"};
    //assert(bmp->rgbquad_count() == 0);
    bmp->print_header(std::cout);
    bmp->write_to_file(new_name.c_str());

    return 0;
}
