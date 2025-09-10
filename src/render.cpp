#include <cstdint>
#include <cstdlib>
#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include "render.hpp"
#include "matrix.hpp"

// Framebuffer indices
#define R (0)
#define G (1)
#define B (2)

Render::Render(Scene &scene, int width, int height, int antiAliasingLevel, int jobs) : mScene(scene)
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

    setupImgPlane();
}

Render::~Render()
{
    free(mFb);
}

int Render::run()
{
    // Spawn our rays from the camera.
    // mAntiAliasingLevel^2 rays per pixel.
    std::cout << "Creating rays..." << std::endl;
    mRays.resize(mHeight);
    for (int y = 0; y < mHeight; y++)
    {
        mRays[y].resize(mWidth);
        for (int x = 0; x < mWidth; x++)
        {
            mRays[y][x].resize(mAntiAliasingLevel * mAntiAliasingLevel);
            for (vector_t v : getImgPlanePixelMultiple(y, x))
            {
                vector_t vRay = Matrix::vsub(v, mScene.mCamera.mOrigin);
                mRays[y][x][0] = Ray(mScene.mCamera.mOrigin, Matrix::vnorm(vRay));
            }
        }
    }
    std::cout << "Created " << mRays.size() * mRays[0].size() * mRays[0][0].size() << " rays." << std::endl;
    std::cout << "Starting render with " << mJobs << " threads..." << std::endl;

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

void Render::setupImgPlane()
{
    /**
     * Make the assumption that the x axis of the scene is always
     * parallel to the XZ plane (the floor). aka the camera is always
     * upright.
     *
     * The math:
     * - Direction vector will *always* be normal to the image plane.
     *   The direction vector is also guaranteed to be normalized.
     * - The height vector will *always* point down. We made the assumption
     *   that the camera is upright.
     * - The width vector will *always* be in the XZ plane. The camera is upright.
     * - Goal: Find the vectors for the width and height of the image plane
     *         and the position of the top left corner.
     *
     * We can find the width vector by projecting the dir. vec. into
     * the xz-plane. We then find the vector 90 degrees to the right.
     * That is the width vector. Normalize and scale to match the pixel
     * width.
     *
     * Take the cross product of the direction and width vectors to get
     * the height vector. Check its direction and flip vector signs
     * accordingly (should always point down towards -Y). Normalize
     * and scale to match pixel height.
     *
     * Add everything up to find the top left corner.
     */

    vector_t dirInXZ = mScene.mCamera.mDir;
    dirInXZ[V_Y] = 0;
    mPlaneWidth = Matrix::vscale(Matrix::vnorm({-dirInXZ[V_Z], 0, dirInXZ[V_X]}), mWidth);
    mPlaneHeight = Matrix::vscale(Matrix::vnorm(Matrix::cross3(mScene.mCamera.mDir, mPlaneWidth)), mHeight);
    if (mPlaneHeight[V_Y] > 0.0)
    {
        throw std::logic_error("Height vector points towards +Y");
    }

    vector_t imagePlaneCenter = Matrix::vadd(mScene.mCamera.mOrigin, Matrix::vscale(mScene.mCamera.mDir, mScene.mCamera.mFocalLength));
    vector_t halfWidth = Matrix::vscale(mPlaneWidth, 0.5);
    vector_t halfHeight = Matrix::vscale(mPlaneHeight, 0.5);
    mPlaneOrigin = Matrix::vsub(Matrix::vsub(imagePlaneCenter, halfWidth), halfHeight);
}

vector_t Render::getImgPlanePixel(int y, int x)
{
    vector_t ret = Matrix::vadd(mPlaneOrigin, Matrix::vscale(mPlaneHeight, ((double)y / mHeight) + 0.5));
    return Matrix::vadd(ret, Matrix::vscale(mPlaneWidth, ((double)x / mWidth) + 0.5));
}

matrix_t Render::getImgPlanePixelMultiple(int y, int x)
{
    double step = 1.0 / mAntiAliasingLevel;
    matrix_t ret = matrix_t(mAntiAliasingLevel * mAntiAliasingLevel);
    int i = 0;

    // Make sure our grid is centered on the pixel
    for (double dy = y + 0.5 * step; dy < (y + 0.99); dy += step)
    {
        for (double dx = x + 0.5 * step; dx < (x + 0.99); dx += step)
        {
            vector_t vec = Matrix::vadd(mPlaneOrigin, Matrix::vscale(mPlaneHeight, (double)dy / mHeight));
            ret[i++] = Matrix::vadd(vec, Matrix::vscale(mPlaneWidth, (double)dx / mWidth));
        }
    }
    return ret;
}