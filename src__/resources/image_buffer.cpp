#include "image_buffer.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

ImageBuffer::ImageBuffer(const std::string& filename) : _data(nullptr), _width(0), _height(0), _channels(0) {
    // Load the image using stb_image.
    _data = stbi_load(filename.c_str(), &_width, &_height, &_channels, 0);
    if (!_data) {
        throw std::runtime_error("Failed to load image: " + filename);
    }
}

ImageBuffer::~ImageBuffer() {
    if (_data) {
        stbi_image_free(_data);
        _data = nullptr;
    }
}

ImageBuffer::ImageBuffer(ImageBuffer&& other) noexcept
    : _data(other._data), _width(other._width), _height(other._height), _channels(other._channels) {
    // Reset the source object to avoid double-free.
    other._data = nullptr;
    other._width = 0;
    other._height = 0;
    other._channels = 0;
}

ImageBuffer& ImageBuffer::operator=(ImageBuffer&& other) noexcept {
    if (this != &other) {
        if (_data) {
            stbi_image_free(_data);
        }
        _data = other._data;
        _width = other._width;
        _height = other._height;
        _channels = other._channels;

        // Reset the source object.
        other._data = nullptr;
        other._width = 0;
        other._height = 0;
        other._channels = 0;
    }
    return *this;
}

unsigned char* ImageBuffer::data() const { return _data; }

int ImageBuffer::width() const { return _width; }

int ImageBuffer::height() const { return _height; }

int ImageBuffer::channels() const { return _channels; }
