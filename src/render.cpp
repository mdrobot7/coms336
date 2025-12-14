#include <cstdint>
#include <cstdlib>
#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <ctime>
#include <limits>
#include "render.hpp"
#include "vector.hpp"
#include "bvh.hpp"

// Framebuffer indices
#define R (0)
#define G (1)
#define B (2)

Render::Render(Scene &scene, BoundingVolumeHierarchy &bvh, int width, int height, int antiAliasingLevel, int jobs, int maxBounces) : mScene(scene), mBvh(bvh)
{
    mWidth = width;
    mHeight = height;
    mAntiAliasingLevel = antiAliasingLevel;
    mFb = (uint8_t *)malloc(width * height * 3); // 24 bit color, 8 bits per channel
    if (!mFb)
    {
        throw std::bad_alloc();
    }

    mJobs = jobs;
    mNextPixelX = mNextPixelY = 0;
    mKillThreads = false;

    mMaxBounces = maxBounces;

    srand(time(NULL));

    setupImgPlane();
    Vector focalLength = Vector::svscale(mScene.mCamera.mFront, mScene.mCamera.mFocalLength);
    mPinhole = Vector::svsub(mScene.mCamera.mOrigin, focalLength);
}

Render::~Render()
{
    free(mFb);
}

int Render::run()
{
    std::cout << "Using " << mHeight * mWidth * mAntiAliasingLevel << " rays." << std::endl;

    // Create a pool of threads to dispatch jobs to.
    // A job is just a pixel, really. Could even do it
    // by each ray. It doesn't matter what order the pixels
    // are rendered in, just that the final value is
    // written to the framebuffer.
    std::cout << "Starting render with " << mJobs << " threads..." << std::endl;
    for (int i = 0; i < mJobs; i++)
    {
        mThreads.emplace_back(std::thread(&Render::renderPixel, this));
    }

    std::cout << "Started threads. Rendering..." << std::endl;
    while (true)
    {
        mNextPixelLock.lock();
        int nextX = mNextPixelX;
        int nextY = mNextPixelY;
        mNextPixelLock.unlock();

        // Lovely progress bar
        const int barWidth = 70;
        const double progress = (((double)nextY * mWidth + nextX) / (mWidth * mHeight));
        std::cout << "[";
        int pos = barWidth * progress;
        for (int i = 0; i < barWidth; ++i)
        {
            if (i < pos)
                std::cout << "=";
            else if (i == pos)
                std::cout << ">";
            else
                std::cout << " ";
        }
        std::cout << "] " << int(progress * 100.0) << " %\r";
        std::cout.flush();

        if (progress == 1)
        {
            for (int i = 0; i < mJobs; i++)
            {
                mThreads[i].join();
            }
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    std::cout << std::endl;

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

void Render::renderPixel()
{
    while (!mKillThreads)
    {
        mNextPixelLock.lock();
        int nextX = mNextPixelX;
        int nextY = mNextPixelY;
        if (nextY >= mHeight)
        {
            mNextPixelLock.unlock();
            break;
        }
        else
        {
            mNextPixelX = (mNextPixelX + 1) % mWidth;
            if (mNextPixelX == 0 && mNextPixelY < mHeight)
            {
                mNextPixelY++;
            }
        }
        mNextPixelLock.unlock();

        Color pixelColor = {0.0, 0.0, 0.0};
        for (int i = 0; i < mAntiAliasingLevel; i++)
        {
            Vector v = getImgPlanePixelRandom(nextY, nextX);
            Ray inRay = Ray(v, Vector::svsub(v, mPinhole).vnorm());

            // Trace the ray. Keep tracing until we run out of bounces, miss everything, or we get absorbed.
            for (int j = 0; j < mMaxBounces; j++)
            {
                // Check BVH
                Ray outRay;
                double t = std::numeric_limits<double>::infinity();
                Color color;
                object::Primitive::Collision collision = mBvh.intersects(inRay, outRay, t, color);
                inRay = Ray(outRay);

                if (color.closeToZero())
                {
                    // Call the pixel black and move on, no point in simulating anything else
                    break;
                }

                if (collision == object::Primitive::Collision::REFLECTED)
                {
                    // We have more stuff to hit
                    inRay.addCollision(color);
                }
                else if (collision == object::Primitive::Collision::ABSORBED)
                {
                    // Ray was absorbed, we've found its final color
                    inRay.addCollision(color);
                    pixelColor.vadd(inRay.mColor);
                    break;
                }
                else if (collision == object::Primitive::Collision::MISSED)
                {
                    // Missed everything, meaning we never hit a light and
                    // got absorbed. Give up and leave the pixel black
                    break;
                }
            }
        }
        pixelColor.vscale(1.0 / mAntiAliasingLevel); // Average our ray colors

        mFbLock.lock();
        uint8_t *pixel = getPixel(nextY, nextX);
        pixel[R] = (uint8_t)(pixelColor[R] * 255);
        pixel[G] = (uint8_t)(pixelColor[G] * 255);
        pixel[B] = (uint8_t)(pixelColor[B] * 255);
        mFbLock.unlock();
    }
}

uint8_t *Render::getPixel(int y, int x)
{
    if (y < 0 || y >= mHeight || x < 0 || x >= mWidth)
    {
        throw std::invalid_argument("Invalid coordinate value");
    }
    return mFb + (y * mWidth + x) * 3;
}

void Render::setupImgPlane()
{
    /**
     * The math:
     * - Direction vector will *always* be normal to the image plane.
     *   The direction vector is also guaranteed to be normalized.
     * - Top vector will *always* be in the plane. Also guaranteed to be normalized.
     * - Top vector will *always* be parallel to the height vector.
     * - Goal: Find the vectors for the width and height of the image plane
     *         and the position of the top left corner.
     *
     * The direction and top vectors define the image plane, it's just not in
     * the right spot and not a fixed size. We're also missing the width vector.
     *
     * Width vector can be found with a cross product, then it's just a matter of
     * translating the plane into place to find the top left corner. It's centered
     * at the origin now, account for the image plane size, focal length, and camera
     * position.
     *
     * The width and height vectors should be the width and height of one pixel. The image
     * plane is aspectRatio x 1 "unit", so normalize and divide by the resolution.
     */

    double aspectRatio = (double)mWidth / mHeight;

    mPlaneHeight.vscale(mScene.mCamera.mTop, -1.0 / mHeight);
    Vector widthNorm = Vector::svnorm(Vector::scross3(mScene.mCamera.mFront, mScene.mCamera.mTop));
    mPlaneWidth.vscale(widthNorm, aspectRatio / mWidth);

    Vector halfTop = Vector::svscale(mScene.mCamera.mTop, 0.5);
    Vector halfWidth = Vector::svscale(widthNorm, 0.5 * aspectRatio);
    mPlaneOrigin.vadd(mScene.mCamera.mOrigin, Vector::svsub(halfTop, halfWidth));
}

Vector &Render::getImgPlanePixel(int y, int x)
{
    Vector ret = Vector::svadd(mPlaneOrigin, Vector::svscale(mPlaneHeight, y + 0.5));
    return ret.vadd(ret, Vector::svscale(mPlaneWidth, x + 0.5));
}

Vector Render::getImgPlanePixelRandom(int y, int x)
{
    double dx = x + (rand() / ((double)RAND_MAX + 1));
    double dy = y + (rand() / ((double)RAND_MAX + 1));
    Vector ret = Vector::svadd(mPlaneOrigin, Vector::svscale(mPlaneHeight, dy));
    ret.vadd(Vector::svscale(mPlaneWidth, dx));
    return ret;
}

void Render::getImgPlanePixelMultiple(Vector vectors[], int y, int x)
{
    for (int i = 0; i < mAntiAliasingLevel; i++)
    {
        double dx = x + (rand() / ((double)RAND_MAX + 1));
        double dy = y + (rand() / ((double)RAND_MAX + 1));
        vectors[i] = Vector::svadd(mPlaneOrigin, Vector::svscale(mPlaneHeight, dy));
        vectors[i].vadd(Vector::svscale(mPlaneWidth, dx));
    }
}