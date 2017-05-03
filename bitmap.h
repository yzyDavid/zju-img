//
// Created by yzy on 3/17/17.
//

// GNU compiler and C++11 is needed.

#ifndef HW1_BITMAP_H
#define HW1_BITMAP_H

#include <cstdint>
#include <memory>
#include <fstream>
#include <iostream>
#include <cassert>
#include <type_traits>
#include <cstring>
#include <utility>
#include <functional>

#include "yuv_image.h"
#include "public_flags.h"
#include "basic_image.h"
#include "kernel.h"

namespace wheel
{
    // only 24-bit or 32-bit bitmap should be supported.
    // also i do not wanna handle situations when headers are not well set.
    class bitmap final : public basic_image
    {
    public:
        struct bitmap_file_header_type
        {
            friend std::ostream &operator<<(std::ostream &os, const bitmap_file_header_type &type);

            uint16_t type;
            uint32_t size;
            uint16_t reserved1;
            uint16_t reserved2;
            uint32_t offset_bytes;
        }__attribute__((packed));

        static_assert(sizeof(bitmap_file_header_type) == 14);
        static_assert(std::is_pod<bitmap_file_header_type>::value);

        struct bitmap_info_header_type
        {
            friend std::ostream &operator<<(std::ostream &os, const bitmap_info_header_type &type);

            uint32_t size;
            uint32_t width;
            int32_t height;
            uint16_t planes;
            uint16_t bit_count;
            uint32_t compression;
            uint32_t size_image;
            uint32_t x_pixels_per_meter;
            uint32_t y_pixels_per_meter;
            uint32_t color_used;
            uint32_t color_important;
        }__attribute__((packed));

        static_assert(sizeof(bitmap_info_header_type) == 40);
        static_assert(std::is_pod<bitmap_info_header_type>::value);

        struct bitmap_rgbquad_type
        {
            friend std::ostream &operator<<(std::ostream &os, const bitmap_rgbquad_type &type);

            uint8_t blue;
            uint8_t green;
            uint8_t red;
            uint8_t reserved;
        }__attribute__((packed));

        static_assert(sizeof(bitmap_rgbquad_type) == 4);
        static_assert(std::is_pod<bitmap_rgbquad_type>::value);

        struct rgb_pixel
        {
            uint8_t blue;
            uint8_t green;
            uint8_t red;
        }__attribute((packed));

        static_assert(sizeof(rgb_pixel) == 3);
        static_assert(std::is_pod<rgb_pixel>::value);

    private:
        bitmap_file_header_type file_header;
        bitmap_info_header_type info_header;

        bitmap_rgbquad_type *rgbquads;

        uint8_t *data;

    private:
        size_t file_size;

    public:
        ~bitmap()
        {
            delete[] data;
            delete[] rgbquads;
        }

        bitmap(const bitmap &that)
                : file_header(that.file_header),
                  info_header(that.info_header),
                  file_size(that.file_size),
                  data(new uint8_t[that.data_size()]),
                  rgbquads(new bitmap_rgbquad_type[that.rgbquad_count()])
        {
            memcpy(data, that.data, that.data_size());
            memcpy(rgbquads, that.rgbquads, sizeof(bitmap_rgbquad_type) * that.rgbquad_count());
        }

        bitmap(bitmap &&that) noexcept
                : file_header(std::move(that.file_header)),
                  info_header(std::move(that.info_header)),
                  file_size(that.file_size),
                  data(that.data),
                  rgbquads(that.rgbquads)
        {}

    public:
        void for_each(std::function<void(rgb_pixel& px)> op);

        std::shared_ptr<bitmap> binarize(uint8_t threshold, bool inverse = false) const;

        template<typename TElement, uint8_t height, uint8_t width>
        std::shared_ptr<bitmap> convolution2d(const kernel<TElement, height, width> &ker) const
        {
            auto re = std::shared_ptr<bitmap>(new bitmap(*this));
            auto img_height = re->info_header.height;
            auto img_width = re->info_header.width;
            auto c = ker.center;

            for (int i = 0; i < img_height; ++i)
            {
                for (int j = 0; j < img_width; ++j)
                {
                    uint8_t r = 0, g = 0, b = 0;
                    for (int m = std::max(0, (int) (i - c.y));
                         m < std::min(img_height, (int) (i + height - c.y)); ++m)
                    {
                        for (int n = std::max(0, (int) (j - c.x));
                             n < std::min(img_width, j + width - c.x); ++n)
                        {
                            b += ker.content[i - m][j - n].blue * position_ro(m, n).blue;
                            g += ker.content[i - m][j - n].green * position_ro(m, n).green;
                            r += ker.content[i - m][j - n].red * position_ro(m, n).red;
                        }
                    }
                    re->position(i, j) = rgb_pixel{b, g, r};
                }
            }

            return re;
        }

        template<uint8_t kernel_order>
        std::shared_ptr<bitmap> mean_filter() const
        {
            const int width = kernel_order, height = kernel_order;
            auto ker = kernel<bitmap::rgb_pixel, width, height>{};
            for (int i = 0; i < width; ++i)
            {
                for (int j = 0; j < height; ++j)
                {
                    ker.content[i][j] = rgb_pixel{255, 255, 255};
                }
            }
            ker.center = {(uint32_t) (kernel_order / 2), (uint32_t) (kernel_order / 2)};

            auto re = std::shared_ptr<bitmap>(new bitmap(*this));
            auto img_height = re->info_header.height;
            auto img_width = re->info_header.width;
            auto c = ker.center;

            for (int i = 0; i < img_height; ++i)
            {
                for (int j = 0; j < img_width; ++j)
                {
                    uint16_t r = 0, g = 0, b = 0;
                    uint8_t times = 0;
                    for (int m = std::max(0, (int) (i - c.y)); m < std::min(img_height, (int) (i + height - c.y)); ++m)
                    {
                        for (int n = std::max(0, (int) (j - c.x));
                             n < std::min(img_width, j + width - c.x); ++n)
                        {
                            b += position_ro(m, n).blue;
                            g += position_ro(m, n).green;
                            r += position_ro(m, n).red;
                            ++times;
                        }
                    }
                    re->position(i, j) = rgb_pixel{(uint8_t) (b / times), (uint8_t) (g / times), (uint8_t) (r / times)};
                }
            }

            return re;
        }

        template<typename TElement, uint8_t height, uint8_t width>
        std::shared_ptr<bitmap> erode(const kernel<TElement, height, width> &ker) const
        {
            auto re = std::shared_ptr<bitmap>(new bitmap(*this));
            auto img_height = re->info_header.height;
            auto img_width = re->info_header.width;
            auto c = ker.center;

            for (int i = 0; i < img_height; ++i)
            {
                for (int j = 0; j < img_width; ++j)
                {
                    bool flag = true;
                    for (int m = std::max(0, (int) (i - c.y)); m < std::min(img_height, (int) (i + height - c.y)); ++m)
                    {
                        for (int n = std::max(0, (int) (j - c.x));
                             n < std::min(img_width, j + width - c.x); ++n)
                        {
                            if (ker.content[i - m][j - n].blue > 0 && position_ro(m, n).blue < 250)
                            {
                                flag = false;
                            }
                        }
                    }
                    re->position(i, j) = flag ? rgb_pixel{255, 255, 255} : rgb_pixel{0, 0, 0};
                }
            }

            return re;
        }

        template<typename TElement, uint8_t height, uint8_t width>
        std::shared_ptr<bitmap> dilate(const kernel<TElement, height, width> &ker) const
        {
            auto re = std::shared_ptr<bitmap>(new bitmap(*this));
            auto img_height = re->info_header.height;
            auto img_width = re->info_header.width;
            auto c = ker.center;

            for (int i = 0; i < img_height; ++i)
            {
                for (int j = 0; j < img_width; ++j)
                {
                    bool flag = false;
                    for (int m = std::max(0, (int) (i - c.y)); m < std::min(img_height, (int) (i + height - c.y)); ++m)
                    {
                        for (int n = std::max(0, (int) (j - c.x));
                             n < std::min(img_width, j + width - c.x); ++n)
                        {
                            if (ker.content[i - m][j - n].blue > 0 && position_ro(m, n).blue > 250)
                            {
                                flag = true;
                            }
                        }
                    }
                    re->position(i, j) = flag ? rgb_pixel{255, 255, 255} : rgb_pixel{0, 0, 0};
                }
            }

            return re;
        }

        template<typename TElement, uint8_t height, uint8_t width>
        std::shared_ptr<bitmap> opening(const kernel<TElement, height, width> &ker) const
        {
            auto r1 = erode(ker);
            return r1->dilate(ker);
        };

        template<typename TElement, uint8_t height, uint8_t width>
        std::shared_ptr<bitmap> closing(const kernel<TElement, height, width> &ker) const
        {
            auto r1 = dilate(ker);
            return r1->erode(ker);
        };

    public:
        std::shared_ptr<bitmap> dump_header() const
        {
            auto res = std::shared_ptr<bitmap>(new bitmap{});
            res->file_header = this->file_header;
            res->info_header = this->info_header;
            res->file_size = this->file_size;
            res->data = new uint8_t[res->data_size()];
            res->rgbquads = new bitmap_rgbquad_type[res->rgbquad_count()];
            memcpy(res->rgbquads, this->rgbquads, sizeof(bitmap_rgbquad_type) * res->rgbquad_count());
            return res;
        }

        std::shared_ptr<bitmap> resize_and_empty(uint32_t width, int32_t height) const;

        std::shared_ptr<bitmap> resize(uint32_t width, int32_t height) const;

        std::shared_ptr<bitmap> translate(uint32_t x, uint32_t y) const;

        std::shared_ptr<bitmap> mirror(bool is_around_x = true) const;

        std::shared_ptr<bitmap> rotate(double rad) const;

        std::shared_ptr<bitmap> scale(double rate) const;

        std::shared_ptr<bitmap> shear(double d, bool is_shear_on_x = true) const;

        rgb_pixel &operator[](size_t index)
        {
            return at(index);
        }

        rgb_pixel &at(size_t index)
        {
            size_t row = index / line_size();
            size_t column = index - row * info_header.width;
            return position(row, column);
        }

        const rgb_pixel &at_ro(size_t index) const
        {
            size_t row = index / line_size();
            size_t column = index - row * info_header.width;
            return position_ro(row, column);
        }

        rgb_pixel &position(size_t row, size_t column)
        {
            size_t offset = row * line_size() + column * pixel_size();
            auto addr = reinterpret_cast<rgb_pixel *>(reinterpret_cast<uintptr_t>(data) + offset);
            return *addr;
        }

        const rgb_pixel &position_ro(size_t row, size_t column) const
        {
            size_t offset = row * line_size() + column * pixel_size();
            auto addr = reinterpret_cast<rgb_pixel *>(reinterpret_cast<uintptr_t>(data) + offset);
            return *addr;
        }

        size_t data_size() const
        {
            //return (info_header.bit_count ? info_header.bit_count : 24) / 8 * info_header.height * info_header.width;
            auto size = file_size - sizeof(bitmap_file_header_type) - sizeof(bitmap_info_header_type)
                        - sizeof(bitmap_rgbquad_type) * rgbquad_count();
            assert(line_size() * info_header.height == size);
            return size;
        }

        size_t pixel_size() const
        {
            size_t s = static_cast<size_t>(info_header.bit_count / 8);
            size_t calculated = data_size() / (info_header.height * info_header.width);
            assert(s == calculated);
            return s;
        }

        size_t rgbquad_count() const
        {
            assert((file_header.offset_bytes - sizeof(bitmap_file_header_type) - sizeof(bitmap_info_header_type))
                   % sizeof(bitmap_rgbquad_type) == 0);
            return (file_header.offset_bytes - sizeof(bitmap_file_header_type) - sizeof(bitmap_info_header_type))
                   / sizeof(bitmap_rgbquad_type);
        }

    private:
        bitmap()
                : data(nullptr),
                  rgbquads(nullptr)
        {}

    private:
        size_t line_size() const
        {
            auto capacity = info_header.bit_count / 8 * info_header.width;
            return capacity % 4 == 0 ? capacity : (capacity + 4) - (capacity % 4);
        }

        bool assert_headers()
        {
            if (info_header.bit_count != 24 && info_header.bit_count != 32)
            {
                info_header.bit_count = 24;
            }
            return !(info_header.height == 0 || info_header.width == 0);
        }

    public:
        static std::shared_ptr<bitmap> from_file(const char *filename);

        static std::shared_ptr<bitmap> from_yuv(std::shared_ptr<yuv_image> img, std::shared_ptr<bitmap> header_from);

        void write_to_file(const char *filename) const;

        std::shared_ptr<yuv_image> to_yuv() const;

        void print_header(std::ostream &) const;

        friend std::ostream &operator<<(std::ostream &os, const bitmap::bitmap_info_header_type &type);

        friend std::ostream &operator<<(std::ostream &os, const bitmap::bitmap_file_header_type &type);

        friend std::ostream &wheel::operator<<(std::ostream &os, const bitmap::bitmap_rgbquad_type &type);
    };
}


#endif //HW1_BITMAP_H
