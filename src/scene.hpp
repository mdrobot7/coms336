#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include "tiny_obj_loader.h"
#include "matrix.hpp"
#include "ray.hpp"

typedef std::vector<uint8_t> color_t;

namespace object
{
    class Primitive
    {
    public:
        static Ray collide(Ray incoming);
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

        static Ray collide(Ray incoming);
    };

    /**
     * @brief Simple sphere class.
     * Complex objects should be imported using an obj file.
     *
     */
    class Sphere : public Primitive
    {
    public:
        vector_t mCenter;
        double mRadius;
        bool mSpectral; // True for spectral (shiny), false for diffuse (dull)
        color_t mColor;

        Sphere(vector_t center, double radius, bool spectral, color_t color);

        static Ray collide(Ray incoming);
    };

    class Light : public Primitive
    {
    public:
        vector_t mCenter;
        color_t mColor;

        Light(vector_t center, color_t color);

        static Ray collide(Ray incoming);
    };

    class Camera
    {
    public:
        vector_t mOrigin;
        vector_t mDir;
        double mFocalLength;

        Camera(vector_t origin, vector_t dir, double focalLength);
    };
}; // namespace Object

class Scene
{
public:
    // List of primitive shapes, used for simple geometry
    std::vector<object::Primitive> mPrimitives;

    // Attributes pulled from OBJ files, used for complex geometry
    tinyobj::attrib_t mAttributes;
    std::vector<tinyobj::shape_t> mShapes;
    std::vector<tinyobj::material_t> mMaterials;

    /**
     * @brief Load the scene data from a JSON file and
     * populate all of the objects at their correct coordinates.
     *
     * @param sceneJsonPath
     * @return int
     */
    int load(std::string sceneJsonPath);

private:
};
