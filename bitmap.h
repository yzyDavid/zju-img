//
// Created by yzy on 3/17/17.
//

// GNU compiler and C++11 is needed.

#ifndef HW1_BITMAP_H
#define HW1_BITMAP_H

#include <cstdint>
#include <memory>
#include <fstream>
#include "yuv_image.h"

namespace wheel
{
    // only 24-bit or 32-bit bitmap should be supported.
    // also i do not wanna handle situations when headers are not well set.
    class bitmap final
    {
    public:
        struct bitmap_file_header_type
        {
            uint16_t type;
            uint32_t size;
            uint16_t reserved1;
            uint16_t reserved2;
            uint32_t offset_bytes;
        }__attribute__((packed));

        struct bitmap_info_header_type
        {
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

        struct bitmap_rgbquad_type
        {
            uint8_t blue;
            uint8_t green;
            uint8_t red;
            uint8_t reserved;
        }__attribute__((packed));

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

        size_t data_size()
        {
/*            return info_header.bit_count / 8 * info_header.height * info_header.width;*/
            return file_size - sizeof(bitmap_file_header_type) - sizeof(bitmap_info_header_type)
                   - sizeof(bitmap_rgbquad_type) * rgbquad_count();
        }

        size_t rgbquad_count()
        {
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
                return false;
            if (info_header.height == 0 || info_header.width == 0)
                return false;
            return true;
        }

    public:
        static std::shared_ptr<bitmap> from_file(char *filename);

        static std::shared_ptr<bitmap> from_yuv(std::shared_ptr<yuv_image> *img);

        void write_to_file(char *filename);

        std::shared_ptr<yuv_image> to_yuv();
    };
}


#endif //HW1_BITMAP_H
