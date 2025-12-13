#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include "scene.hpp"
#include "ray.hpp"
#include "bvh.hpp"

class Render
{
public:
    Render(Scene &scene, BoundingVolumeHierarchy &bvh, int width, int height, int antiAliasingLevel, int jobs, int maxBounces);
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

    BoundingVolumeHierarchy &mBvh;

    int mWidth, mHeight, mAntiAliasingLevel;
    uint8_t *mFb;
    std::mutex mFbLock;

    int mJobs;
    std::vector<std::thread> mThreads;
    int mNextPixelX, mNextPixelY;
    std::mutex mNextPixelLock;
    bool mKillThreads;

    int mMaxBounces; // Max bounces per ray before we call it black

    Vector mPlaneWidth, mPlaneHeight, mPlaneOrigin; // Width/heights are normalized, origin is top left corner
    Vector mPinhole;                                // Location of the pinhole camera

    /**
     * @brief Job for an individual thread in the thread
     * pool. Renders the next pixel in the queue and puts
     * the result in the framebuffer.
     */
    void renderPixel();

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
    Vector &getImgPlanePixel(int y, int x);

    /**
     * @brief Get a vector that points to a random location in
     * the pixel in the image plane specified by x, y.
     */
    Vector getImgPlanePixelRandom(int y, int x);

    /**
     * @brief Get a set of mAntiAliasingLevel vectors randomly
     * distributed in the pixel of the image plane specified
     * by x, y.
     *
     * @return matrix_t A mAntiAliasingLevel x 3 matrix of row vectors
     */
    void getImgPlanePixelMultiple(Vector vectors[], int y, int x);
};