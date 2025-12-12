#include "stb.hpp"

#include <stdexcept>

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
    if (y >= mHeight || x >= mWidth)
    {
        throw std::invalid_argument("Indices out of range.");
    }
    unsigned char *offset = mImage + y * mHeight + x * mChannels;
    return (Color)(Vector(offset[0], offset[1], offset[2]).vnorm());
}

Color STBImage::getUv(double u, double v)
{
    return get(u * (mWidth - 1), v * (mHeight - 1));
}

unsigned char STBImage::get(int y, int x, int color)
{
    if (y >= mHeight || x >= mWidth || color >= mChannels)
    {
        throw std::invalid_argument("Indices out of range.");
    }
    unsigned char *offset = mImage + y * mHeight + x * mChannels + color;
    return *offset;
}

double STBImage::getDbl(int y, int x, int color)
{
    if (y >= mHeight || x >= mWidth || color >= mChannels)
    {
        throw std::invalid_argument("Indices out of range.");
    }
    unsigned char *offset = mImage + y * mHeight + x * mChannels + color;
    return *offset / 255.0;
}