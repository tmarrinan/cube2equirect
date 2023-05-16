#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"

uint8_t* iioReadImage(const char *filename, int *width, int *height, int *channels)
{
    return stbi_load(filename, width, height, channels, *channels);
}

void iioFreeImage(uint8_t *image)
{
    stbi_image_free(image);
}

int iioWriteImageJpeg(const char *filename, int width, int height, int channels, int quality, uint8_t *pixels)
{
    return stbi_write_jpg(filename, width, height, channels, pixels, quality);
}

int ioWriteImagePng(const char *filename, int width, int height, int channels, uint8_t *pixels)
{
    return stbi_write_png(filename, width, height, channels, pixels, width * channels);
}

