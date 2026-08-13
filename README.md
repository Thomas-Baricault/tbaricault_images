# tbaricault/images

[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
![C++23](https://img.shields.io/badge/C%2B%2B-23-blue)
![CMake](https://img.shields.io/badge/CMake-3.20%2B-blue)

## Description

This is a C++23 library providing an `Image` class that allows to manipulate images and image files.

## Table of Contents

- [Description](#description)
- [Features](#features)
- [Requirements](#requirements)
- [Usage](#usage)
    - [Download and install](#download-and-install)
    - [Uninstall](#uninstall)
    - [CMake](#cmake)
    - [Include](#include)
    - [Environment](#environment)
- [Documentation](#documentation)
- [Examples](#examples)
    - [Load and save image](#load-and-save-image)
- [License](#license)

## Features

- Image class
- Load image from file
- Save image to file

## Requirements

- C++23 or later
- CMake 3.20 or later
- [libjpeg-turbo/libjpeg-turbo](https://github.com/libjpeg-turbo/libjpeg-turbo)
- [pnggroup/libpng](https://github.com/pnggroup/libpng)
- [tbaricault/colors](https://github.com/Thomas-Baricault/tbaricault_colors)
- [tbaricault/filesystem](https://github.com/Thomas-Baricault/tbaricault_filesystem)
- [tbaricault/math](https://github.com/Thomas-Baricault/tbaricault_math)

## Usage

### Download and install

```bash
git clone https://github.com/Thomas-Baricault/tbaricault_images.git
cd tbaricault_images
make install
```

### Uninstall

```bash
make uninstall
```

### CMake

Add the library to your project:

```cmake
find_package(tbaricault_images REQUIRED)

target_link_libraries(
    my_target
    PRIVATE
        tbaricault::images
)
```

### Include

```cpp
#include <tbaricault/images.hpp>
```

### Environment

If you have a custom C++ installation, you can edit the `ENV` variable in the `Makefile` to specify your environment path.

Example on Windows with MSYS2/MinGW64:

```makefile
ENV = C:/msys64/mingw64
```

## Documentation

Read the complete documentation at [https://docs.thomas-baricault.fr/images](https://docs.thomas-baricault.fr/images).

## Examples

### Load and save image

```cpp
#include <iostream>
#include <tbaricault/images.hpp>


int main()
{
    tbaricault::images::Image img("my_image.png");
    if (img)
        img.save("dest.jpeg");
    else
        std::cout << "Image not found" << std::endl;

    return (0);
}
```

## License

This project is licensed under the MIT License.

See [LICENSE](LICENSE) for details.
