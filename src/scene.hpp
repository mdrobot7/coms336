#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include "nlohmann/json.hpp"
#include "tiny_obj_loader.h"
#include "matrix.hpp"
#include "ray.hpp"

typedef std::vector<uint8_t> color_t;

static inline color_t intToColor(int i)
{
    return color_t{(uint8_t)((i & 0xFF0000) >> 16), (uint8_t)((i & 0xFF00) >> 8), (uint8_t)(i & 0xFF)};
}

static inline int colorToInt(color_t c)
{
    return (c[0] << 16) | (c[1] << 8) | c[2];
}

namespace object
{
    class Primitive
    {
    public:
        Primitive();
        Primitive(nlohmann::json json);
        Ray collide(Ray incoming);
    };

    /**
     * @brief Simple triangle class.
     * Complex objects (i.e. with textures) should be
     * imported using an obj file.
     *
     */
    class Triangle : public Primitive
    {
    public:
        matrix_t mVertices; // Vertices are row vectors
        bool mSpectral;     // True for spectral (shiny), false for diffuse (dull)
        color_t mColor;

        Triangle(vector_t v0, vector_t v1, vector_t v2, bool spectral, color_t color);
        Triangle(nlohmann::json json);

        Ray collide(Ray incoming);
    };

    /**
     * @brief Simple sphere class.
     * Complex objects should be imported using an obj file.
     *
     */
    class Sphere : public Primitive
    {
    public:
        vector_t mOrigin;
        double mRadius;
        bool mSpectral; // True for spectral (shiny), false for diffuse (dull)
        color_t mColor;

        Sphere(vector_t origin, double radius, bool spectral, color_t color);
        Sphere(nlohmann::json json);

        Ray collide(Ray incoming);
    };

    class Light : public Primitive
    {
    public:
        vector_t mOrigin;
        color_t mColor;

        Light(vector_t origin, color_t color);
        Light(nlohmann::json json);

        Ray collide(Ray incoming);
    };

    class Model : public Primitive
    {
    public:
        vector_t mOrigin;
        vector_t mFront;
        vector_t mTop;
        vector_t mScale; // [scaleX, scaleY, scaleZ]

        tinyobj::ObjReader &mObj;

        Model(tinyobj::ObjReader obj, vector_t origin, vector_t front, vector_t top, vector_t scale);
        Model(nlohmann::json json, tinyobj::ObjReader obj);

        Ray collide(Ray incoming);
    };

    class Camera
    {
    public:
        vector_t mOrigin;
        vector_t mFront;
        vector_t mTop;
        double mFocalLength;

        Camera();
        Camera(vector_t origin, vector_t front, vector_t top, double focalLength);
        Camera(nlohmann::json json);
    };
}; // namespace Object

class Scene
{
public:
    object::Camera mCamera;

    // List of scene objects
    std::vector<object::Primitive> mPrimitives;

    // Dedicated list for lights, we'll be using this a lot
    std::vector<object::Light> mLights;

    // List of OBJ files, containing their assets
    std::vector<tinyobj::ObjReader> mObjReaders;
    std::vector<std::string> mObjFilenames;

    Scene();

    /**
     * @brief Load the scene data from a JSON file and
     * populate all of the objects at their correct coordinates.
     *
     * @param sceneJsonPath
     */
    void load(std::string sceneJsonPath);

private:
};
