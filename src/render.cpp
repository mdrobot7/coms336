#include <cstdint>
#include <cstdlib>
#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <ctime>
#include "render.hpp"
#include "matrix.hpp"

// Framebuffer indices
#define R (0)
#define G (1)
#define B (2)

Render::Render(const Scene &scene, int width, int height, int antiAliasingLevel, int jobs, int maxBounces) : mScene(scene)
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
}

Render::~Render()
{
    free(mFb);
}

int Render::run()
{
    // Spawn our rays from the camera.
    // mAntiAliasingLevel rays per pixel.
    std::cout << "Creating rays..." << std::endl;
    mRays.resize(mHeight);
    for (int y = 0; y < mHeight; y++)
    {
        mRays[y].resize(mWidth);
        for (int x = 0; x < mWidth; x++)
        {
            mRays[y][x].resize(mAntiAliasingLevel);
            matrix_t imagePlaneVecs = getImgPlanePixelMultiple(y, x);
            for (int i = 0; i < mAntiAliasingLevel; i++)
            {
                vector_t vRay = Matrix::vsub(imagePlaneVecs[i], mScene.mCamera.mOrigin);
                mRays[y][x][i] = Ray(mScene.mCamera.mOrigin, Matrix::vnorm(vRay));
            }
        }
    }
    std::cout << "Created " << mRays.size() * mRays[0].size() * mRays[0][0].size() << " rays." << std::endl;

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
    std::vector<Ray> rays;
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

        mRaysLock.lock();
        rays = mRays[nextY][nextX];
        mRaysLock.unlock();

        color_t pixelColor = {0.0, 0.0, 0.0};
        for (Ray r : rays)
        {
            // Trace the ray. Keep tracing until we run out of bounces, miss everything, or we get absorbed.
            object::Primitive::Collision collision = object::Primitive::Collision::REFLECTED;
            for (int i = 0; i < mMaxBounces && collision == object::Primitive::Collision::REFLECTED; i++)
            {
                for (const auto &p : mScene.mPrimitives)
                {
                    collision = p->collide(r);
                    if (collision != object::Primitive::Collision::MISSED)
                    {
                        break;
                    }
                }
            }
            pixelColor = Matrix::vadd(pixelColor, r.mColor); // TEMP: Grab whatever color the ray is, I have no skybox
            if (collision == object::Primitive::Collision::ABSORBED)
            {
                // If the ray is absorbed, we've found its color. Otherwise,
                // it's black (don't need to add it, it's just adding 0).
                // pixelColor = Matrix::vadd(pixelColor, r.mColor);
            }
        }
        pixelColor = Matrix::vscale(pixelColor, 1.0 / mAntiAliasingLevel); // Average our ray colors

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
     */

    mPlaneHeight = Matrix::vscale(mScene.mCamera.mTop, -1);
    mPlaneWidth = Matrix::vnorm(Matrix::cross3(mScene.mCamera.mFront, mScene.mCamera.mTop));

    vector_t focalLength = Matrix::vscale(mScene.mCamera.mFront, mScene.mCamera.mFocalLength);
    vector_t halfHeight = Matrix::vscale(mPlaneHeight, mHeight * 0.5);
    vector_t halfWidth = Matrix::vscale(mPlaneWidth, mWidth * 0.5);
    mPlaneOrigin = Matrix::vadd(mScene.mCamera.mOrigin, Matrix::vsub(focalLength, Matrix::vadd(halfWidth, halfHeight)));
}

vector_t Render::getImgPlanePixel(int y, int x)
{
    vector_t ret = Matrix::vadd(mPlaneOrigin, Matrix::vscale(mPlaneHeight, y + 0.5));
    return Matrix::vadd(ret, Matrix::vscale(mPlaneWidth, x + 0.5));
}

matrix_t Render::getImgPlanePixelMultiple(int y, int x)
{
    matrix_t ret = matrix_t(mAntiAliasingLevel);

    for (int i = 0; i < mAntiAliasingLevel; i++)
    {
        double dx = x + (rand() / ((double)RAND_MAX + 1));
        double dy = y + (rand() / ((double)RAND_MAX + 1));
        vector_t vec = Matrix::vadd(mPlaneOrigin, Matrix::vscale(mPlaneHeight, dy));
        ret[i] = Matrix::vadd(vec, Matrix::vscale(mPlaneWidth, dx));
    }
    return ret;
}