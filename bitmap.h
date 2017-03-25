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

#include "yuv_image.h"
#include "public_flags.h"

namespace wheel
{
    // only 24-bit or 32-bit bitmap should be supported.
    // also i do not wanna handle situations when headers are not well set.
    class bitmap final
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

        bitmap(const bitmap &) = delete;

        bitmap(bitmap &&) = delete;

        size_t data_size() const
        {
            return (info_header.bit_count ? info_header.bit_count : 24) / 8 * info_header.height * info_header.width;
            //return file_size - sizeof(bitmap_file_header_type) - sizeof(bitmap_info_header_type)
            //       - sizeof(bitmap_rgbquad_type) * rgbquad_count();
        }

        size_t pixel_size() const
        {
            return data_size() / (info_header.height * info_header.width);
        }

        size_t rgbquad_count() const
        {
            assert((file_header.offset_bytes - sizeof(bitmap_file_header_type) - sizeof(bitmap_info_header_type))
                   % sizeof(bitmap_rgbquad_type) == 0);
            return (file_header.offset_bytes - sizeof(bitmap_file_header_type) - sizeof(bitmap_info_header_type))
                   / sizeof(bitmap_rgbquad_type);
        }

        bitmap()
                : data(nullptr),
                  rgbquads(nullptr)
        {}

    private:
        bool assert_headers()
        {
            if (info_header.bit_count != 24 && info_header.bit_count != 32)
            {
                info_header.bit_count = 24;
            }
            return !(info_header.height == 0 || info_header.width == 0);
        }

    public:
        static std::shared_ptr<bitmap> from_file(char *filename);

        static std::shared_ptr<bitmap> from_yuv(std::shared_ptr<yuv_image> img);

        void write_to_file(const char *filename) const;

        std::shared_ptr<yuv_image> to_yuv() const;

        void print_header(std::ostream &) const;

        friend std::ostream &operator<<(std::ostream &os, const bitmap::bitmap_info_header_type &type);

        friend std::ostream &operator<<(std::ostream &os, const bitmap::bitmap_file_header_type &type);

        friend std::ostream &wheel::operator<<(std::ostream &os, const bitmap::bitmap_rgbquad_type &type);
    };
}


#endif //HW1_BITMAP_H
