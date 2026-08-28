/*
 * Copyright (c) 2026-present Thomas Baricault
 *
 * SPDX-License-Identifier: MIT
 */


#pragma once


#include <string>

#include <tbaricault/colors.hpp>
#include <tbaricault/math.hpp>


namespace tbaricault::images
{

    /**
     * @brief 2D image stored in RGBA format
     * 
     * @note This class depends on libpng and libjpeg for encoding/decoding
     */
    class Image final
    {

        public:

            /**
             * @brief Image format
             */
            enum class Format
            {

                /**
                 * @brief Automatically resolve image format
                 */
                Auto = 0,

#ifdef _WIN32
                /**
                 * @brief Bitmap format
                 */
                BMP = 1,
#endif

                /**
                 * @brief JPEG format
                 */
                JPEG = 2,

                /**
                 * @brief PNG format
                 */
                PNG = 3,

            };


            /**
             * @brief Constructs an invalid image
             */
            Image() noexcept = default;

            /**
             * @brief Copy constructor
             * 
             * @param other Image to copy
             */
            Image(const Image& other);

            /**
             * @brief Move constructor
             * 
             * @param other Image to move
             */
            Image(Image&& other) noexcept;

            /**
             * @brief Constructs an image from raw pixel data
             * 
             * @param size Image size (width, height)
             * @param data Pointer to pixel data (RGBA format)
             * @param copy If `true`, the data is copied, otherwise, ownership is taken
             */
            Image(const tbaricault::math::Vector2<int>& size, tbaricault::colors::RGBA* data, bool copy = true);

            /**
             * @brief Loads an image from a file
             * 
             * @param path Path to the image file
             * @param format File format
             */
            Image(const std::string& path, Format format = Format::Auto);

            /**
             * @brief Destructor
             */
            ~Image() noexcept;

            /**
             * @brief Copy assignment operator
             * 
             * @param other Image to copy
             * 
             * @return Reference to this image
             */
            Image& operator=(const Image& other);

            /**
             * @brief Move assignement operator
             * 
             * @param other Image to move
             * 
             * @return Reference to this image
             */
            Image& operator=(Image&& other) noexcept;

            /**
             * @brief Returns whether the image is in a valid state
             */
            explicit operator bool() const noexcept;

            /**
             * @brief Returns the image dimensions
             * 
             * @return Image size (width, height)
             */
            const tbaricault::math::Vector2<int>& getSize() const noexcept;

            /**
             * @brief Returns a pointer to the pixel buffer
             * 
             * @return Pointer to RGBA pixel data
             */
            tbaricault::colors::RGBA* getPixels() const noexcept;

            /**
             * @brief Saves the image to a file
             * 
             * @param path Output file path
             * @param format Ouput file format
             * 
             * @return `true` on success, `false` otherwise
             */
            bool save(const std::string& path, Format format = Format::Auto) const;


        protected:

            /**
             * @brief Image size
             */
            tbaricault::math::Vector2<int> _size;

            /**
             * @brief Image pixels data
             */
            tbaricault::colors::RGBA* _data = nullptr;

    };

}
