#include <cstdint>
#include <cstdlib>
#include <string>
#include <fstream>
#include <iostream>
#include "render.hpp"

// Framebuffer indices
#define R (0)
#define G (1)
#define B (2)

Render::Render(int width, int height, int jobs)
{
    mWidth = width;
    mHeight = height;
    mFb = (uint8_t *)malloc(width * height * 3); // 24 bit color, 8 bits per channel
    if (!mFb)
    {
        throw std::bad_alloc();
    }

    mJobs = jobs;
}

Render::~Render()
{
    free(mFb);
}

int Render::run()
{
    for (int y = 0; y < mHeight; y++)
    {
        for (int x = 0; x < mWidth; x++)
        {
            getPixel(y, x)[R] = (256 * x) / mWidth;
            getPixel(y, x)[G] = 0;
            getPixel(y, x)[B] = 0;
        }
    }
    return 0;
}

int Render::save(std::string filename)
{
    std::ofstream out;

    // Binary
    out.open(filename + ".ppm", std::ios::out | std::ios::binary);
    out << "P6\n"
        << mWidth << " " << mHeight << "\n255\n";
    out.write((const char *)mFb, mWidth * mHeight * 3);
    out.close();

    // ASCII
    out.open(filename + ".txt.ppm");
    out << "P3\n"
        << mWidth << " " << mHeight << "\n255\n";
    for (int y = 0; y < mHeight; y++)
    {
        for (int x = 0; x < mWidth; x++)
        {
            // int casts are to persuade CPP into printing these values as integers
            out << (int)(getPixel(y, x)[R]) << " "
                << (int)(getPixel(y, x)[G]) << " "
                << (int)(getPixel(y, x)[B]) << "\n";
        }
    }
    out.close();
    return 0;
}

uint8_t *Render::getPixel(int y, int x)
{
    if (y < 0 || y >= mHeight || x < 0 || x >= mWidth)
    {
        throw std::invalid_argument("Invalid coordinate value");
    }
    return mFb + (y * mWidth + x) * 3;
}