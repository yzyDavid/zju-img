//
// Created by yzy on 3/17/17.
//

#include <stdexcept>
#include <fstream>
#include <cassert>
#include <cmath>
#include <functional>

#include "bitmap.h"
#include "public_flags.h"

namespace wheel
{
    std::shared_ptr<bitmap> bitmap::from_file(const char *filename)
    {
        std::ifstream ifs(filename);
        if (!ifs.is_open())
            throw std::runtime_error("opening file failed");

        ifs.seekg(0, std::ios::end);
        auto length = ifs.tellg();
        ifs.seekg(0, std::ios::beg);
        if (length < sizeof(bitmap_file_header_type) + sizeof(bitmap_info_header_type))
            throw std::runtime_error("invalid bitmap file length");

        auto bmp = std::shared_ptr<bitmap>(new bitmap{});
        bmp->file_size = (size_t) length;

        ifs.read(reinterpret_cast<char *>(&bmp->file_header), sizeof(bitmap::bitmap_file_header_type));
        ifs.read(reinterpret_cast<char *>(&bmp->info_header), sizeof(bitmap::bitmap_info_header_type));

        if (!bmp->assert_headers())
            throw std::runtime_error("invalid bitmap file content");

        if (bmp->info_header.bit_count != 24 && bmp->info_header.bit_count != 32)
            throw std::runtime_error("NOT IMPLEMENTED");

        if (bmp->rgbquad_count())
        {
            bmp->rgbquads = new bitmap_rgbquad_type[bmp->rgbquad_count()];
            ifs.read(reinterpret_cast<char *>(bmp->rgbquads), bmp->rgbquad_count() * sizeof(bitmap_rgbquad_type));
            //std::clog << bmp->rgbquad_count() << std::endl;
        }

        bmp->data = new uint8_t[bmp->data_size()];
        ifs.read(reinterpret_cast<char *>(bmp->data), bmp->data_size());
        ifs.close();

#ifdef WHEEL_DEBUG
        bmp->print_header(std::cout);
#endif

        return bmp;
    }

    void bitmap::write_to_file(const char *filename) const
    {
        std::ofstream ofs(filename);
        if (!ofs.is_open())
            throw std::runtime_error("opening file failed");
        ofs.write(reinterpret_cast<const char *>(&file_header), sizeof(file_header));
        ofs.flush();
        ofs.write(reinterpret_cast<const char *>(&info_header), sizeof(info_header));
        ofs.flush();
        ofs.write(reinterpret_cast<const char *>(rgbquads), sizeof(bitmap_rgbquad_type) * rgbquad_count());
        ofs.flush();
        assert(sizeof(file_header) == 14);
        assert(sizeof(info_header) == 40);
        ofs.write(reinterpret_cast<const char *>(data), data_size());
        ofs.flush();

#ifdef WHEEL_DEBUG
        for (auto i = 0; i < 100; ++i)
        {
            std::cout << static_cast<short>(data[i]) << " ";
            if (i % 16 == 15) std::cout << std::endl;
        }
        std::cout.flush();
#endif

        ofs.close();
    }

    std::shared_ptr<yuv_image> bitmap::to_yuv() const
    {
        auto res = std::make_shared<yuv_image>(
                static_cast<uint32_t>(info_header.height < 0 ? -info_header.height : info_header.height),
                static_cast<uint32_t>(info_header.width)
        );

        for (size_t i = 0; i < res->meta.height * res->meta.width; ++i)
        {
            auto p = at_ro(i);
            res->at(i).y = static_cast<double>(0.299 * p.red + 0.587 * p.green + 0.114 * p.blue);
            res->at(i).u = static_cast<double>(0.492 * (p.blue - res->at(i).y));
            res->at(i).v = static_cast<double>(0.877 * (p.red - res->at(i).y));
        }

        return res;
    }

    std::shared_ptr<bitmap> bitmap::from_yuv(std::shared_ptr<yuv_image> img, std::shared_ptr<bitmap> header_from)
    {
        auto res = header_from->dump_header();
        res->info_header.height = img->meta.height;
        res->info_header.width = img->meta.width;

        for (size_t i = 0; i < res->info_header.height * res->info_header.width; ++i)
        {
            auto &p = res->at(i);
            auto f = img->at(i);
            p.red = static_cast<uint8_t>(clamp(f.y + 1.14 * f.v));
            p.green = static_cast<uint8_t>(clamp(f.y - 0.395 * f.u - 0.581 * f.v));
            p.blue = static_cast<uint8_t>(clamp(f.y + 2.033 * f.u));
        }
        return res;
    }

    void bitmap::print_header(std::ostream &os) const
    {
        os << "=== file header ===" << std::endl;
        os << file_header << std::endl;

        os << "=== info header ===" << std::endl;
        os << info_header << std::endl;

        os << "========" << std::endl;
        os << "data size: " << data_size() << std::endl;
        os << "rgbquad count: " << rgbquad_count() << std::endl;
        os << "pixel size: " << pixel_size() << std::endl;

        os << "=== rgb quads ===" << std::endl;
        for (auto i = 0; i < rgbquad_count(); ++i)
        {
            os << rgbquads[i] << std::endl;
        }

        os << std::endl;
    }

    std::ostream &operator<<(std::ostream &os, const bitmap::bitmap_info_header_type &type)
    {
        os << "size: " << type.size << " width: " << type.width << " height: " << type.height << " planes: "
           << type.planes << " bit_count: " << type.bit_count << " compression: " << type.compression << " size_image: "
           << type.size_image << " x_pixels_per_meter: " << type.x_pixels_per_meter << " y_pixels_per_meter: "
           << type.y_pixels_per_meter << " color_used: " << type.color_used << " color_important: "
           << type.color_important;
        return os;
    }

    std::ostream &operator<<(std::ostream &os, const bitmap::bitmap_file_header_type &type)
    {
        os << "type: " << type.type << " size: " << type.size << " reserved1: " << type.reserved1 << " reserved2: "
           << type.reserved2 << " offset_bytes: " << type.offset_bytes;
        return os;
    }

    std::ostream &operator<<(std::ostream &os, const bitmap::bitmap_rgbquad_type &type)
    {
        os << "blue: " << (unsigned int) type.blue
           << " green: " << (unsigned int) type.green
           << " red: " << (unsigned int) type.red
           << " reserved: " << (unsigned int) type.reserved;
        return os;
    }

    std::shared_ptr<bitmap> bitmap::binarize(uint8_t threshold, bool inverse) const
    {
        auto re = std::shared_ptr<bitmap>(new bitmap(*this));

        for (size_t i = 0; i < re->info_header.width * re->info_header.height; ++i)
        {
            re->at(i).red = re->at(i).green = re->at(i).blue = (uint8_t) (
                    ((re->at_ro(i).red + re->at_ro(i).green + re->at_ro(i).blue) / 3 >= threshold) ^ inverse ?
                    255 : 0);
        }

        return re;
    }

    std::shared_ptr<bitmap> bitmap::resize_and_empty(uint32_t width, int32_t height) const
    {
        auto res = std::shared_ptr<bitmap>(new bitmap());
        res->info_header = this->info_header;
        res->file_header = this->file_header;

        res->info_header.height = height;
        res->info_header.width = width;

        res->file_size = sizeof(bitmap_file_header_type) + sizeof(bitmap_info_header_type)
                         + sizeof(bitmap_rgbquad_type) * res->rgbquad_count() +
                         res->line_size() * res->info_header.height;

        res->data = new uint8_t[res->data_size()];
        res->rgbquads = new bitmap_rgbquad_type[res->rgbquad_count()];

        memcpy(res->rgbquads, rgbquads, sizeof(bitmap_rgbquad_type) * res->rgbquad_count());
        memset(res->data, 0, res->data_size());

        return res;
    }

    std::shared_ptr<bitmap> bitmap::resize(uint32_t width, int32_t height) const
    {
        auto res = this->resize_and_empty(width, height);
        auto t_height = this->info_header.height < 0 ? -this->info_header.height : this->info_header.height;

        for (int i = 0; i < this->info_header.width; ++i)
        {
            for (int j = 0; j < t_height; ++j)
            {
                res->position(j, i) = this->position_ro(j, i);
            }
        }
        return res;
    }

    std::shared_ptr<bitmap> bitmap::translate(uint32_t x, uint32_t y) const
    {
        auto res = this->resize_and_empty(info_header.width + x, info_header.height + y);

        auto t_height = this->info_header.height < 0 ? -this->info_header.height : this->info_header.height;

        for (int i = 0; i < this->info_header.width; ++i)
        {
            for (int j = 0; j < t_height; ++j)
            {
                res->position(j + y, i + x) = this->position_ro(j, i);
            }
        }

        return res;
    }

    std::shared_ptr<bitmap> bitmap::mirror(bool is_around_x) const
    {
        auto res = std::shared_ptr<bitmap>();
        auto t_height = this->info_header.height < 0 ? -this->info_header.height : this->info_header.height;

        if (is_around_x)
        {
            res = this->resize_and_empty(this->info_header.width, this->info_header.height * 2);
            for (int i = 0; i < this->info_header.width; ++i)
            {
                for (int j = 0; j < t_height; ++j)
                {
                    res->position(2 * t_height - j - 1, i) = this->position_ro(j, i);
                }
            }
        } else
        {
            res = this->resize_and_empty(this->info_header.width * 2, this->info_header.height);
            for (int i = 0; i < this->info_header.width; ++i)
            {
                for (int j = 0; j < t_height; ++j)
                {
                    res->position(j, 2 * this->info_header.width - i - 1) = this->position_ro(j, i);
                }
            }
        }

        return res;
    }

    /*
     * x0 = xcos + ysin
     * y0 = ycos - xsin
     */
    std::shared_ptr<bitmap> bitmap::rotate(double rad) const
    {
        double s = sin(rad);
        double c = cos(rad);

        auto h = this->info_header.height;
        auto w = this->info_header.width;

        size_t nh, nw;

        auto res = this->resize_and_empty((nw = info_header.width * (1 + sin(rad)) + 1),
                                          (nh = info_header.height * (1 + sin(rad)) + 1));

        auto t_height = this->info_header.height < 0 ? -this->info_header.height : this->info_header.height;

        for (int i = 0; i < w; ++i)
        {
            for (int j = 0; j < h; ++j)
            {
                int32_t y = round(j * cos(rad) + i * sin(rad));
                int32_t x = round(i * cos(rad) - j * sin(rad));
                if (x < 0 || x >= w - 1)x = INT32_MIN;
                if (y < 0 || y >= h - 1)y = INT32_MIN;
                if (x != INT32_MIN && y != INT32_MIN)
                {
                    res->position(j + s * h, i + s * w) = this->position_ro(x - s, y - s);
                } else
                {
                    res->position(j + s * h, i + s * w) = {0, 0, 0};
                }
            }
        }

        return res;
    }

    std::shared_ptr<bitmap> bitmap::scale(double rate) const
    {
        auto h = this->info_header.height;
        auto w = this->info_header.width;
        auto res = this->resize_and_empty(w * rate, h * rate);

        auto t_height = this->info_header.height < 0 ? -this->info_header.height : this->info_header.height;

        for (int i = 0; i < w * rate; ++i)
        {
            for (int j = 0; j < h * rate; ++j)
            {
                res->position(j, i) = this->position_ro(round(j / rate), round(i / rate));
            }
        }

        return res;
    }

    std::shared_ptr<bitmap> bitmap::shear(double d, bool is_shear_on_x) const
    {
        auto h = this->info_header.height;
        auto w = this->info_header.width;
        auto t_height = this->info_header.height < 0 ? -this->info_header.height : this->info_header.height;

        auto res = std::shared_ptr<bitmap>();

        if (is_shear_on_x)
        {
            res = this->resize_and_empty(w + d * h, h);

            for (int i = 0; i < w + d * h; ++i)
            {
                for (int j = 0; j < h; ++j)
                {
                    if (i - (1 + d) * j <= 0 || i - (1 + d) * j > w)
                        res->position(j, i) = {0, 0, 0};
                    else
                        res->position(j, i) = this->position_ro(round(j), round(i - (1 + d) * j));
                }
            }
        } else
        {
            res = this->resize_and_empty(w, h + d * w);

            for (int i = 0; i < w; ++i)
            {
                for (int j = 0; j < h + d * w; ++j)
                {
                    if (j - (1 + d) * i <= 0 || j - (1 + d) * i > h)
                        res->position(j, i) = {0, 0, 0};
                    else
                        res->position(j, i) = this->position_ro(round(j - (1 + d) * i), round(i));
                }
            }
        }

        return res;
    }

    void bitmap::for_each(std::function<void(bitmap::rgb_pixel &)> op)
    {
        for (size_t i = 0; i < info_header.width * info_header.height; ++i)
        {
            op(at(i));
        }
    }

    std::shared_ptr<bitmap> bitmap::bilaterial_filter(double sigma_d, double sigma_r) const
    {
        auto re = std::shared_ptr<bitmap>(new bitmap(*this));
        const unsigned int height = 11, width = 11;
        kernel<float64_pixel, height, width> ker;
        ker.center = {5, 5};
        auto img_height = re->info_header.height;
        auto img_width = re->info_header.width;
        auto c = ker.center;

        for (int i = 0; i < img_height; ++i)
        {
            for (int j = 0; j < img_width; ++j)
            {
                double r = 0, g = 0, b = 0;
                double sw_r = 0, sw_g = 0, sw_b = 0;

                for (int m = std::max(0, (int) (i - c.y));
                     m < std::min(img_height, (int) (i + height - c.y)); ++m)
                {
                    for (int n = std::max(0, (int) (j - c.x));
                         n < std::min(img_width, j + width - c.x); ++n)
                    {
                        double w1 = ((i - m) * (i - m) + (j - n) * (j - n)) / (2 * sigma_d * sigma_d);
                        double w2_b =
                                (std::abs(position_ro(i, j).blue - position_ro(m, n).blue)) / (2 * sigma_r * sigma_r);
                        double w2_g =
                                (std::abs(position_ro(i, j).green - position_ro(m, n).green)) / (2 * sigma_r * sigma_r);
                        double w2_r =
                                (std::abs(position_ro(i, j).red - position_ro(m, n).red)) / (2 * sigma_r * sigma_r);
                        double e_b = std::exp(-w1 - w2_b);
                        double e_g = std::exp(-w1 - w2_g);
                        double e_r = std::exp(-w1 - w2_r);
                        r += position_ro(m, n).red * e_r;
                        g += position_ro(m, n).green * e_g;
                        b += position_ro(m, n).blue * e_b;
                        sw_r += e_r;
                        sw_g += e_g;
                        sw_b += e_b;
                    }
                }
                b /= sw_b;
                g /= sw_g;
                r /= sw_r;
                re->position(i, j) = rgb_pixel{b, g, r};
            }
        }

        return re;
    }
}
