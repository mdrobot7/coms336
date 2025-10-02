#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include "nlohmann/json.hpp"
#include "tiny_obj_loader.h"
#include "matrix.hpp"
#include "ray.hpp"
#include "color.hpp"

namespace object
{
    class Primitive
    {
    public:
        enum Collision
        {
            REFLECTED = 0,
            ABSORBED,
            MISSED,
        };

        Primitive();
        Primitive(nlohmann::json json);

        virtual ~Primitive() {};

        /**
         * @brief Collide a ray with this object.
         *
         * @param incoming Incoming ray
         * @return enum Collision Type of collision that occurred
         */
        virtual enum Collision collide(Ray &incoming) const = 0;

        // Ray collision helpers (common to all object types)

        /**
         * @brief Handle a specular reflection.
         *
         * @param incoming Incoming ray to be reflected
         * @param intersection Intersection point
         * @param normal Normal vector of collision surface
         * @return true If the ray should keep bouncing
         * @return false If the ray has been absorbed
         */
        bool specular(Ray &incoming, vector_t intersection, vector_t normal) const;

        /**
         * @brief Handle a diffuse reflection.
         *
         * @param incoming Incoming ray to be reflected
         * @param intersection Intersection point
         * @param normal Normal vector of collision surface
         * @return true If the ray should keep bouncing
         * @return false If the ray has been absorbed
         */
        bool diffuse(Ray &incoming, vector_t intersection, vector_t normal) const;

        /**
         * @brief Handle a dielectric reflection/refraction.
         *
         * @param incoming Incoming ray to be reflected.
         * @return true If the ray should keep bouncing
         * @return false If the ray has been absorbed
         */
        bool dielectric(Ray &incoming) const;
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
        vector_t mNormal;   // Normal vector, determined by winding order of vertices
        enum Color::Surface mSurface;
        color_t mColor;

        Triangle(vector_t v0, vector_t v1, vector_t v2, enum Color::Surface surface, color_t color);
        Triangle(nlohmann::json json);

        enum Collision collide(Ray &incoming) const override;
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
        enum Color::Surface mSurface;
        color_t mColor;

        Sphere(vector_t origin, double radius, enum Color::Surface surface, color_t color);
        Sphere(nlohmann::json json);

        enum Collision collide(Ray &incoming) const override;
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

        enum Collision collide(Ray &incoming) const override;
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

    // List of scene objects. Must be unique_ptr otherwise polymorphism breaks
    std::vector<std::unique_ptr<object::Primitive>> mPrimitives;

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
