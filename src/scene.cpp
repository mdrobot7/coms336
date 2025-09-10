#include "scene.hpp"

namespace object
{
    Ray Primitive::collide(Ray incoming)
    {
        return incoming;
    }

    Triangle::Triangle(vector_t v0, vector_t v1, vector_t v2, bool spectral, color_t color)
    {
        mVertices = matrix_t(3);
        mVertices[0] = v0;
        mVertices[0] = v1;
        mVertices[0] = v2;
        mSpectral = spectral;
        mColor = color;
    }

    Ray Triangle::collide(Ray incoming)
    {
        // TODO
        return incoming;
    }

    Sphere::Sphere(vector_t center, double radius, bool spectral, color_t color)
    {
        mCenter = center;
        mRadius = radius;
        mSpectral = spectral;
        mColor = color;
    }

    Ray Sphere::collide(Ray incoming)
    {
        // TODO
        return incoming;
    }

    Light::Light(vector_t center, color_t color)
    {
        mCenter = center;
        mColor = color;
    }

    Camera::Camera(vector_t origin, vector_t dir, double focalLength)
    {
        mOrigin = origin;
        mDir = dir;
        mFocalLength = focalLength;
    }
}

int Scene::load(std::string sceneJsonPath)
{
}