#pragma once

#include <cstdint>
#include <string>

class Render
{
public:
    Render(int width, int height, int jobs);
    ~Render();

    /**
     * @brief Render the scene.
     *
     * @return int 0 on success, nonzero otherwise
     */
    int run();

    /**
     * @brief Saves the rendered framebuffer to two PPM
     * files: filename.ppm (binary) and filename.txt.ppm
     * (ASCII).
     *
     * @param filename
     * @return int
     */
    int save(std::string filename);

private:
    int mWidth, mHeight;
    uint8_t *mFb;

    int mJobs;

    uint8_t *getPixel(int y, int x);
};