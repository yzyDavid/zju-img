//
// Created by yzy on 3/17/17.
//

#include <stdexcept>
#include <fstream>
#include <cassert>

#include "bitmap.h"
#include "public_flags.h"

namespace wheel
{
    std::shared_ptr<bitmap> bitmap::from_file(char *filename)
    {
        std::ifstream ifs(filename);
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
        ifs.read(reinterpret_cast<char *>(&bmp->info_header), sizeof(bitmap::bitmap_info_header_type));

        if (!bmp->assert_headers())
            throw std::runtime_error("invalid bitmap file content");

/*        if (bmp->info_header.bit_count != 24)
            throw std::runtime_error("NOT IMPLEMENTED");*/

        if (bmp->rgbquad_count())
        {
            bmp->rgbquads = new bitmap_rgbquad_type[bmp->rgbquad_count()];
            ifs.read(reinterpret_cast<char *>(bmp->rgbquads), bmp->rgbquad_count() * sizeof(bitmap_rgbquad_type));
            //std::clog << bmp->rgbquad_count() << std::endl;
        }

        bmp->data = new uint8_t[bmp->data_size()];
        ifs.read(reinterpret_cast<char *>(bmp->data), bmp->data_size());
        ifs.close();
        bmp->print_header(std::cout);

        //reset offset bytes to ensure it can be write back to file correctly.
        //comment these lines, for do not modify the file header, just write them back as what they like originally.
/*        bmp->file_size -= bmp->rgbquad_count() * sizeof(bitmap_rgbquad_type);
        bmp->file_header.offset_bytes = 54;
        bmp->info_header.size = 40;
        auto old_size = bmp->file_header.size;
        bmp->file_header.size = static_cast<uint32_t>(54 + bmp->info_header.height * bmp->info_header.width *
                                                           bmp->pixel_size());*/
        //std::clog << old_size << " " << bmp->file_header.size;

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

        for (auto i = 0; i < res->meta.height * res->meta.width; ++i)
        {
            res->data[i].y;
        }

        return res;
    }

    std::shared_ptr<bitmap> bitmap::from_yuv(std::shared_ptr<yuv_image> img)
    {
        auto res = std::make_shared<bitmap>();
        res->info_header.height = img->meta.height;
        res->info_header.width = img->meta.width;

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
}
