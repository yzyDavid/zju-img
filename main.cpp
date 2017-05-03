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
#include <cmath>

#include "bitmap.h"
#include "yuv_image.h"
#include "kernel.h"

void hw2_entry(const char *filename);

void hw3_entry(const char *filename);

void hw4_entry(const char *filename);

void hw5_entry(const char *filename);

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

    hw5_entry(filename);

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

    auto bin = bmp->binarize(128);
    bin->write_to_file((std::string(filename) + ".bin.bmp").data());

    auto er = bin->erode(ker);
    er->write_to_file((std::string(filename) + ".erode.bmp").data());

    auto di = bin->dilate(ker);
    di->write_to_file((std::string(filename) + ".dilate.bmp").data());

    auto op = bin->opening(ker);
    op->write_to_file((std::string(filename) + ".opening.bmp").data());

    auto cl = bin->closing(ker);
    cl->write_to_file((std::string(filename) + ".closing.bmp").data());
}

void hw3_entry(const char *filename)
{
    using namespace std;
    using namespace wheel;

    auto bmp = bitmap::from_file(filename);

    auto loged = bmp->to_yuv();
    loged = loged->logarithmic();
    auto loged_bmp = bitmap::from_yuv(loged, bmp);
    loged_bmp->write_to_file((std::string(filename) + ".log.bmp").data());

    auto histo = bmp->to_yuv();
    histo = histo->histogram_equalize(8);
    auto histo_bmp = bitmap::from_yuv(histo, bmp);
    histo_bmp->write_to_file((std::string(filename) + ".histo.bmp").data());
}

void hw4_entry(const char *filename)
{
    using namespace std;
    using namespace wheel;

    auto bmp = bitmap::from_file(filename);

    auto resized = bmp->resize(2000, 2000);
    resized->write_to_file((std::string(filename) + ".resize.bmp").data());

    auto trans = bmp->translate(200, 100);
    trans->write_to_file((std::string(filename) + ".trans.bmp").data());

    auto mirror = bmp->mirror(true);
    mirror->write_to_file((std::string(filename) + ".mirror.bmp").data());

    auto mirror_b = bmp->mirror(false);
    mirror_b->write_to_file((std::string(filename) + ".mirror_b.bmp").data());

    auto rotate = bmp->rotate(M_PI_4);
    rotate->write_to_file((std::string(filename) + ".rotate.bmp").data());

    auto scale = bmp->scale(1.5);
    scale->write_to_file((std::string(filename) + ".scale.bmp").data());

    auto shear = bmp->shear(0.5);
    shear->write_to_file((std::string(filename) + ".shear.bmp").data());
}

void hw5_entry(const char *filename)
{
    using namespace std;
    using namespace wheel;

    auto bmp = bitmap::from_file(filename);
    auto gray = bitmap::from_file("/home/yzy/img/lena.bmp_gray.bmp");

    auto mean = bmp->mean_filter<5>();
    mean->write_to_file((std::string(filename) + ".mean.bmp").data());

    auto ker = kernel<float64_pixel, 3, 3>{};
    for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j) ker.content[i][j] = {-0.05, -0.05, -0.05};
    ker.content[1][1] = {0.4, 0.4, 0.4};
    ker.center = {1, 1};

    auto laplace = gray->convolution2d(ker);
    laplace->for_each([](bitmap::rgb_pixel &px)
                      {
                          px.green = px.red;
                      });
    laplace->write_to_file("/home/yzy/img/lena.laplace.bmp");
}
