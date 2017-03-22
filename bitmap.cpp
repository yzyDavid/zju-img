//
// Created by yzy on 3/17/17.
//

#include <stdexcept>
#include <fstream>

#include "bitmap.h"

namespace wheel
{
    std::shared_ptr<bitmap> bitmap::from_file(char *filename)
    {
        std::ifstream ifs(filename, std::ios::binary);
        if (!ifs.is_open())
            throw std::runtime_error("opening file failed");

        ifs.seekg(0, std::ios::end);
        auto length = ifs.tellg();
        ifs.seekg(0, std::ios::beg);
        if (length < sizeof(bitmap_file_header_type) + sizeof(bitmap_info_header_type))
            throw std::runtime_error("invalid bitmap file length");

        auto bmp = std::make_shared<bitmap>();
        bmp->file_size = (size_t) length;

        ifs.read(reinterpret_cast<char *>(&bmp->file_header), sizeof(bitmap::bitmap_file_header_type));
        ifs.read(reinterpret_cast<char *>(&bmp->info_header), sizeof(bitmap::bitmap_file_header_type));

        ifs.close();
        if (!bmp->assert_headers())
            throw std::runtime_error("invalid bitmap file content");

/*        if (bmp->info_header.bit_count != 24)
            throw std::runtime_error("NOT IMPLEMENTED");*/

        if (bmp->rgbquad_count())
        {
            bmp->rgbquads = new bitmap_rgbquad_type[bmp->rgbquad_count()];
            ifs.read(reinterpret_cast<char *>(bmp->rgbquads), bmp->rgbquad_count() * sizeof(bitmap_rgbquad_type));
        }

        bmp->data = new uint8_t[bmp->data_size()];
        ifs.read(reinterpret_cast<char *>(bmp->data), bmp->data_size());
        return bmp;
    }

    void bitmap::write_to_file(char *filename)
    {
        std::ofstream ofs(filename, std::ios::binary);
        if (!ofs.is_open())
            throw std::runtime_error("opening file failed");
        ofs.write(reinterpret_cast<const char *>(&file_header), sizeof(file_header));
        ofs.write(reinterpret_cast<const char *>(&info_header), sizeof(info_header));
    }

    std::shared_ptr<yuv_image> bitmap::to_yuv()
    {
        auto res = std::make_shared<yuv_image>(
                static_cast<uint32_t>(info_header.height < 0 ? -info_header.height : info_header.height),
                static_cast<uint32_t>(info_header.width)
        );

        return res;
    }

    std::shared_ptr<bitmap> bitmap::from_yuv(std::shared_ptr<yuv_image> img)
    {
        auto res = std::make_shared<bitmap>();
        res->info_header.height = img->meta.height;
        res->info_header.width = img->meta.width;

        return res;
    }
}
