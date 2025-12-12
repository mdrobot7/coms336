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
     * @brief Get a pixel from the image.
     */
    Color get(int y, int x);

    /**
     * @brief Get a pixel from the image using
     * texture (u, v) coordinates. u and v are in the
     * range [0, 1.0].
     */
    Color getUv(double u, double v);

    /**
     * @brief Get a color byte [0-255] from the image.
     * Image is in RGB(A) order.
     */
    unsigned char get(int y, int x, int color);

    /**
     * @brief Get a color double [0-1.0] from the image.
     * Image is in RGB(A) order.
     */
    double getDbl(int y, int x, int color);
};