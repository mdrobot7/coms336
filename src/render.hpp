#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "scene.hpp"
#include "ray.hpp"

class Render
{
public:
    Render(Scene &scene, int width, int height, int jobs);
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
    Scene &mScene;

    int mWidth, mHeight;
    uint8_t *mFb;

    int mJobs;

    std::vector<std::vector<std::vector<Ray>>> mRays; // Vector of Rays for each pixel in the screen
    vector_t mPlaneWidth, mPlaneHeight, mPlaneOrigin; // Origin is top left corner

    uint8_t *getPixel(int y, int x);

    void setupImgPlane();
    vector_t getImgPlanePixel(int y, int x);
};