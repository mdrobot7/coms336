#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "scene.hpp"
#include "ray.hpp"

class Render
{
public:
    Render(Scene &scene, int width, int height, int antiAliasingLevel, int jobs);
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

    int mWidth, mHeight, mAntiAliasingLevel;
    uint8_t *mFb;

    int mJobs;

    std::vector<std::vector<std::vector<Ray>>> mRays; // Vector of Rays for each pixel in the screen
    vector_t mPlaneWidth, mPlaneHeight, mPlaneOrigin; // Origin is top left corner

    /**
     * @brief Get a pointer to the pixel in the framebuffer
     * specified by x, y.
     */
    uint8_t *getPixel(int y, int x);

    /**
     * @brief Set up the vectors associated with the image plane.
     *
     */
    void setupImgPlane();

    /**
     * @brief Get a vector that points to the center of the pixel
     * in the image plane specified by x, y.
     */
    vector_t getImgPlanePixel(int y, int x);

    /**
     * @brief Get a set of vectors distributed in a square grid within
     * the specified pixel. The grid is mAntiAliasingLevel^2.
     *
     * @return matrix_t A mAntiAliasingLevel^2 x 3 matrix of row vectors
     */
    matrix_t getImgPlanePixelMultiple(int y, int x);
};