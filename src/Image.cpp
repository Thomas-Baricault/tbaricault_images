/*
 * Copyright (c) 2026-present Thomas Baricault
 *
 * SPDX-License-Identifier: MIT
 */


#include <cstddef>
#include <cstdio>

#include <jpeglib.h>
#include <png.h>

#include <tbaricault/filesystem.hpp>

#include "images/Image.hpp"


namespace tbaricault::images
{

    Image::Image(const Image& other)
    {
        this->_size = other._size;
        this->_data = new tbaricault::colors::RGBA[this->_size.x * this->_size.y];
        std::copy(other._data, other._data + other._size.x * other._size.y, this->_data);
        return;
    }

    Image::Image(Image&& other) noexcept
        : _size(other._size)
        , _data(other._data)
    {
        other._size = 0;
        other._data = nullptr;
        return;
    }

    Image::Image(const tbaricault::math::Vector2<int>& size, tbaricault::colors::RGBA* data, bool copy)
    {
        if (data)
        {
            this->_size = size;
            if (copy)
            {
                this->_data = new tbaricault::colors::RGBA[this->_size.x * this->_size.y];
                std::copy(data, data + size.x * size.y, this->_data);
            }
            else
                this->_data = data;
        }
        return;
    }

    Image::Image(const std::string& path, Format format)
    {
        if (format == Format::Auto)
        {
            std::string extension = tbaricault::filesystem::extension(path);
#ifdef _WIN32
            if (extension == "bmp")
                format = Format::BMP;
            else
#endif
            if (extension == "jpg" || extension == "jpeg")
                format = Format::JPEG;
            else if (extension == "png")
                format = Format::PNG;
            else
                return;
        }
        switch (format)
        {
            case Format::Auto:
                break;
#ifdef _WIN32
            case Format::BMP:
            {
                HBITMAP hbmp = reinterpret_cast<HBITMAP>(LoadImageA(nullptr, static_cast<LPCSTR>(path.c_str()), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
                HDC hdc = GetDC(nullptr);
                BITMAPINFO bmpInfo = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
                bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                GetDIBits(
                    hdc, hbmp,
                    0, 0,
                    nullptr,
                    &bmpInfo,
                    DIB_RGB_COLORS
                );
                bmpInfo.bmiHeader.biBitCount    = 32;
                bmpInfo.bmiHeader.biCompression = BI_RGB;
                bmpInfo.bmiHeader.biHeight      = abs(bmpInfo.bmiHeader.biHeight);
                this->_size = {
                    bmpInfo.bmiHeader.biWidth,
                    bmpInfo.bmiHeader.biHeight
                };
                BYTE* data = new BYTE[bmpInfo.bmiHeader.biSizeImage];
                GetDIBits(
                    hdc, hbmp,
                    0, this->_size.y,
                    data,
                    &bmpInfo,
                    DIB_RGB_COLORS
                );
                this->_data = new tbaricault::colors::RGBA[this->_size.x * this->_size.y];
                for (int y = 0; y < this->_size.y; y++)
                {
                    for (int x = 0; x < this->_size.x; x++)
                    {
                        int i = (this->_size.x * (this->_size.y - y - 1) + x) * 4;
                        this->_data[this->_size.x * y + x] = (
                            data[i]            +
                            (data[i + 1] << 8)  +
                            (data[i + 2] << 16) +
                            (255 << 24)
                        );
                    }
                }
                ReleaseDC(nullptr, hdc);
                delete[] data;
                break;
            }
#endif
            case Format::JPEG:
            {
                FILE* file = fopen(path.c_str(), "rb");
                if (file == nullptr)
                    break;
                jpeg_error_mgr error;
                jpeg_decompress_struct info;
                info.err = jpeg_std_error(&error);
                jpeg_create_decompress(&info);
                jpeg_stdio_src(&info, file);
                jpeg_read_header(&info, 1);
                jpeg_start_decompress(&info);
                this->_size = {
                    static_cast<int>(info.output_width),
                    static_cast<int>(info.output_height)
                };
                int channels = info.output_components;
                this->_data = new tbaricault::colors::RGBA[this->_size.x * this->_size.y];
                unsigned char* row = new JSAMPLE[this->_size.x * channels];
                for (int y = 0; y < this->_size.y; y++)
                {
                    jpeg_read_scanlines(&info, &row, 1);
                    for (int x = 0; x < this->_size.x; x++)
                    {
                        this->_data[this->_size.x * y + x] = (
                            row[x * channels]            +
                            (row[x * channels + 1] << 8)  +
                            (row[x * channels + 2] << 16) +
                            (255 << 24)
                        );
                    }
                }
                delete[] row;
                jpeg_finish_decompress(&info);
                jpeg_destroy_decompress(&info);
                fclose(file);
                break;
            }
            case Format::PNG:
            {
                png_structp read = png_create_read_struct(
                    PNG_LIBPNG_VER_STRING,
                    nullptr, nullptr, nullptr
                );
                if (!read)
                    return;
                png_infop info = png_create_info_struct(read);
                if (!info)
                {
                    png_destroy_read_struct(&read, nullptr, nullptr);
                    break;
                }
                if (setjmp(png_jmpbuf(read)))
                {
                    png_destroy_read_struct(&read, &info, nullptr);
                    break;
                }
                FILE* file = fopen(path.c_str(), "rb");
                if (file == nullptr)
                    break;
                png_init_io(read, file);
                png_read_info(read, info);
                this->_size = {
                    static_cast<int>(png_get_image_width(read, info)),
                    static_cast<int>(png_get_image_height(read, info))
                };
                png_byte colorType = png_get_color_type(read, info);
                if (colorType != PNG_COLOR_TYPE_RGBA)
                {
                    png_set_add_alpha(read, 0xff, PNG_FILLER_AFTER);
                    switch (colorType)
                    {
                        case (PNG_COLOR_TYPE_GRAY):
                        case (PNG_COLOR_TYPE_GRAY_ALPHA):
                        {
                            png_set_gray_to_rgb(read);
                            break;
                        }
                        case (PNG_COLOR_TYPE_PALETTE):
                        {
                            png_set_palette_to_rgb(read);
                            break;
                        }
                        case (PNG_COLOR_TYPE_RGB):
                        {
                            png_set_filler(read, 0xff, PNG_FILLER_AFTER);
                            break;
                        }
                    }
                    png_read_update_info(read, info);
                }
                png_bytepp data = new png_bytep[this->_size.y];
                for (int y = 0; y < this->_size.y; y++)
                {
                    data[y] = new png_byte[this->_size.x * 4];
                    png_read_row(read, data[y], nullptr);
                }
                this->_data = new tbaricault::colors::RGBA[this->_size.x * this->_size.y];
                for (int y = 0; y < this->_size.y; y++)
                {
                    for (int x = 0; x < this->_size.x; x++)
                    {
                        this->_data[this->_size.x * y + x] = (
                            data[y][x * 4]            +
                            (data[y][x * 4 + 1] << 8)  +
                            (data[y][x * 4 + 2] << 16) +
                            (data[y][x * 4 + 3] << 24)
                        );
                    }
                    delete[] data[y];
                }
                delete[] data;
                png_destroy_read_struct(&read, &info, nullptr);
                fclose(file);
                break;
            }
        }
        return;
    }

    Image::~Image() noexcept
    {
        if (this->_data)
            delete[] this->_data;
        return;
    }

    Image& Image::operator=(const Image& other)
    {
        if (&other == this)
            return (*this);
        if (this->_data)
            delete[] this->_data;
        this->_size = other._size;
        this->_data = new tbaricault::colors::RGBA[this->_size.x * this->_size.y];
        std::copy(other._data, other._data + other._size.x * other._size.y, this->_data);
        return (*this);
    }

    Image& Image::operator=(Image&& other) noexcept
    {
        if (&other == this)
            return (*this);
        if (this->_data)
            delete[] this->_data;
        this->_size = other._size;
        this->_data = other._data;
        other._size = 0;
        other._data = nullptr;
        return (*this);
    }

    Image::operator bool() const noexcept
    {
        return (this->_data != nullptr);
    }

    const tbaricault::math::Vector2<int>& Image::getSize() const noexcept
    {
        return (this->_size);
    }

    tbaricault::colors::RGBA* Image::getPixels() const noexcept
    {
        return (this->_data);
    }

    bool Image::save(const std::string& path, Format format) const
    {
        if (format == Format::Auto)
        {
            std::string extension = tbaricault::filesystem::extension(path);
#ifdef _WIN32
            if (extension == "bmp")
                format = Format::BMP;
            else
#endif
            if (extension == "jpg" || extension == "jpeg")
                format = Format::JPEG;
            else if (extension == "png")
                format = Format::PNG;
            else
                return (false);
        }
        switch (format)
        {
            case Format::Auto:
                return (false);
#ifdef _WIN32
            case Format::BMP:
            {
                if (this->_data == nullptr)
                    return (false);
                FILE* file = fopen(path.c_str(), "wb");
                if (file == nullptr)
                    return (false);
                BYTE* data = new BYTE[this->_size.x * this->_size.y * 4];
                for (int y = 0; y < this->_size.y; y++)
                {
                    for (int x = 0; x < this->_size.x; x++)
                    {
                        data[(this->_size.x * (this->_size.y - y - 1) + x) * 4]     = (this->_data[this->_size.x * y + x] >> 16) & 0xff;
                        data[(this->_size.x * (this->_size.y - y - 1) + x) * 4 + 1] = (this->_data[this->_size.x * y + x] >> 8)  & 0xff;
                        data[(this->_size.x * (this->_size.y - y - 1) + x) * 4 + 2] =  this->_data[this->_size.x * y + x]        & 0xff;
                        data[(this->_size.x * (this->_size.y - y - 1) + x) * 4 + 3] = 0;
                    }
                }
                BITMAPINFOHEADER infoHeader;
                infoHeader.biSize        = sizeof(BITMAPINFOHEADER);
                infoHeader.biSizeImage   = this->_size.x * this->_size.y * 4;
                infoHeader.biWidth       = this->_size.x;
                infoHeader.biHeight      = this->_size.y;
                infoHeader.biPlanes      = 1;
                infoHeader.biBitCount    = 32;
                infoHeader.biCompression = BI_RGB;
                BITMAPFILEHEADER fileHeader;
                fileHeader.bfType      = 'B' + ('M' << 8);
                fileHeader.bfOffBits   = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
                fileHeader.bfSize      = fileHeader.bfOffBits + infoHeader.biSizeImage;
                fileHeader.bfReserved1 = 0;
                fileHeader.bfReserved2 = 0;
                fwrite(&fileHeader, 1, sizeof(BITMAPFILEHEADER), file);
                fwrite(&infoHeader, 1, sizeof(BITMAPINFOHEADER), file);
                fwrite(data, 1, infoHeader.biSizeImage, file);
                fclose(file);
                delete[] data;
                break;
            }
#endif
            case Format::JPEG:
            {
                if (this->_data == nullptr)
                    return (false);
                FILE* file = fopen(path.c_str(), "wb");
                if (file == NULL)
                    return (false);
                jpeg_error_mgr error;
                jpeg_compress_struct info;
                info.err = jpeg_std_error(&error);
                jpeg_create_compress(&info);
                jpeg_stdio_dest(&info, file);
                info.image_width      = this->_size.x;
                info.image_height     = this->_size.y;
                info.input_components = 3;
                info.in_color_space   = JCS_RGB;
                jpeg_set_defaults(&info);
                jpeg_set_quality(&info, 95, 1);
                jpeg_start_compress(&info, 1);
                unsigned char* row = new JSAMPLE[this->_size.x * 3];
                for (int y = 0; y < this->_size.y; y++)
                {
                    for (int x = 0; x < this->_size.x; x++)
                    {
                        row[x * 3]     =  this->_data[this->_size.x * y + x]        & 0xff;
                        row[x * 3 + 1] = (this->_data[this->_size.x * y + x] >> 8)  & 0xff;
                        row[x * 3 + 2] = (this->_data[this->_size.x * y + x] >> 16) & 0xff;
                    }
                    jpeg_write_scanlines(&info, &row, 1);
                }
                delete[] row;
                jpeg_finish_compress(&info);
                jpeg_destroy_compress(&info);
                fclose(file);
                break;
            }
            case Format::PNG:
            {
                if (this->_data == nullptr)
                    return (false);
                png_structp write = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
                if (!write)
                    return (false);
                png_infop info = png_create_info_struct(write);
                if (!info)
                    return (png_destroy_write_struct(&write, nullptr), false);
                if (setjmp(png_jmpbuf(write)))
                    return (png_destroy_write_struct(&write, &info), false);
                FILE* file = fopen(path.c_str(), "wb");
                if (file == nullptr)
                    return (false);
                png_init_io(write, file);
                png_set_IHDR(write, info, this->_size.x, this->_size.y, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
                png_bytepp data = new png_bytep[this->_size.y];
                for (int y = 0; y < this->_size.y; y++)
                {
                    data[y] = new png_byte[this->_size.x * 4];
                    for (int x = 0; x < this->_size.x; x++)
                    {
                        data[y][x * 4]     =  this->_data[this->_size.x * y + x]        & 0xff;
                        data[y][x * 4 + 1] = (this->_data[this->_size.x * y + x] >> 8)  & 0xff;
                        data[y][x * 4 + 2] = (this->_data[this->_size.x * y + x] >> 16) & 0xff;
                        data[y][x * 4 + 3] = (this->_data[this->_size.x * y + x] >> 24) & 0xff;
                    }
                }
                png_set_rows(write, info, const_cast<png_bytepp>(data));
                png_write_png(write, info, PNG_TRANSFORM_IDENTITY, nullptr);
                png_destroy_write_struct(&write, &info);
                fclose(file);
                for (int y = 0; y < this->_size.y; y++)
                    delete[] data[y];
                delete[] data;
                break;
            }
        }
        return (true);
    }

}
