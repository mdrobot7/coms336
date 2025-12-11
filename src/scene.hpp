#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include "nlohmann/json.hpp"
#include "tiny_obj_loader.h"
#include "vector.hpp"
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

        static constexpr double sRefractionGlass = 1.458;

        Primitive();
        Primitive(nlohmann::json json);

        virtual ~Primitive() {};

        /**
         * @brief Collide a ray with this object.
         *
         * @param incoming Incoming ray
         * @param t Time t of collision with the object. Always > 0.
         * @param color Color of the object at the collision point.
         * @return enum Collision Type of collision that occurred
         */
        virtual enum Collision collide(Ray &incoming, double &t, Color &color) const = 0;

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
        bool specular(Ray &incoming, Vector intersection, Vector normal) const;

        /**
         * @brief Handle a diffuse reflection.
         *
         * @param incoming Incoming ray to be reflected
         * @param intersection Intersection point
         * @param normal Normal vector of collision surface
         * @return true If the ray should keep bouncing
         * @return false If the ray has been absorbed
         */
        bool diffuse(Ray &incoming, Vector intersection, Vector normal) const;

        /**
         * @brief Handle a dielectric reflection/refraction.
         *
         * @param incoming Incoming ray to be reflected.
         * @return true If the ray should keep bouncing
         * @return false If the ray has been absorbed
         */
        bool dielectric(Ray &incoming, Vector intersection, Vector normal, double indexOfRefraction) const;
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
        Vector mVertices[3];
        Vector mNormal; // Normal vector, determined by winding order of vertices
        enum Color::Surface mSurface;
        double mIndexOfRefraction;
        Color mColor;

        Triangle(Vector &v0, Vector &v1, Vector &v2, enum Color::Surface surface, double indexOfRefraction, Color &color);
        Triangle(nlohmann::json json);

        enum Collision collide(Ray &incoming, double &t, Color &color) const override;
    };

    /**
     * @brief Simple sphere class.
     * Complex objects should be imported using an obj file.
     *
     */
    class Sphere : public Primitive
    {
    public:
        Vector mOrigin;
        double mRadius;
        enum Color::Surface mSurface;
        double mIndexOfRefraction;
        Color mColor;

        Sphere(Vector &origin, double radius, enum Color::Surface surface, double indexOfRefraction, Color &color);
        Sphere(nlohmann::json json);

        enum Collision collide(Ray &incoming, double &t, Color &color) const override;
    };

    class Quad : public Primitive
    {
    public:
        Vector mOrigin;
        Vector mWidth, mHeight;
        Vector mNormal;
        enum Color::Surface mSurface;
        Color mColor;
        double mIndexOfRefraction;

        Quad(Vector &origin, Vector &width, Vector &height, enum Color::Surface surface, double indexOfRefraction, Color &color);
        Quad(nlohmann::json json);

        enum Collision collide(Ray &incoming, double &t, Color &color) const override;

    private:
        Vector mW; // Used for intersection checking
    };

    class Model : public Primitive
    {
    public:
        Vector mOrigin;
        Vector mFront;
        Vector mTop;
        Vector mScale; // [scaleX, scaleY, scaleZ]

        tinyobj::ObjReader &mObj;

        Model(tinyobj::ObjReader obj, Vector &origin, Vector &front, Vector &top, Vector &scale);
        Model(nlohmann::json json, tinyobj::ObjReader obj);

        enum Collision collide(Ray &incoming, double &t, Color &color) const override;
    };

    /**
     * aspectRatio x 1 "unit" image plane. Origin vector points to the center
     * of the image plane, front and top determine orientation. Focal length
     * sets how far the "pinhole" is behind the image plane.
     */
    class Camera
    {
    public:
        Vector mOrigin;
        Vector mFront;
        Vector mTop;
        double mFocalLength;

        Camera();
        Camera(Vector &origin, Vector &front, Vector &top, double focalLength);
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
