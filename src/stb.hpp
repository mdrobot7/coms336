#pragma once

#include <string>

#include "stb_image.h"
#include "color.hpp"

class STBImage
{
public:
    unsigned char *mImage;
    int mWidth, mHeight, mChannels;

    STBImage();
    STBImage(std::string path);
    ~STBImage();

    /**
     * @brief Get a color vector from the image.
     */
    Color get(int y, int x);

    /**
     * @brief Get a color byte [0-255] from the image.
     * Image is in RGB(A) order.
     */
    unsigned char get(int y, int x, int color);

    /**
     * @brief Get a color double [0-1.0] from the image.
     * Image is in RGB(A) order.
     */
    double get_dbl(int y, int x, int color);
};