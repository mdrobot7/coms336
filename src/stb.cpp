#include "stb.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

STBImage::STBImage() {}

STBImage::STBImage(std::string path)
{
    mImage = stbi_load(path.c_str(), &mWidth, &mHeight, &mChannels, 0);
}

STBImage::~STBImage()
{
    stbi_image_free(mImage);
}

Color STBImage::get(int y, int x)
{
    unsigned char *offset = mImage + y * mHeight + x * mChannels;
    return (Color)(Vector(offset[0], offset[1], offset[2]).vnorm());
}

unsigned char STBImage::get(int y, int x, int color)
{
    unsigned char *offset = mImage + y * mHeight + x * mChannels + color;
    return *offset;
}

double STBImage::get_dbl(int y, int x, int color)
{
    unsigned char *offset = mImage + y * mHeight + x * mChannels + color;
    return *offset / 255.0;
}