#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include "nlohmann/json.hpp"
#include "tiny_obj_loader.h"
#include "stb.hpp"
#include "vector.hpp"
#include "ray.hpp"
#include "color.hpp"
#include "boundingBox.hpp"
#include "perlin.hpp"

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
        static double sEmissiveGain; // Boost light brightness to a max of (sEmissiveGain * [1, 1, 1])

        enum Color::Surface mSurface;
        double mIndexOfRefraction;
        Color mColor;
        STBImage *mTexture;
        BoundingBox mBoundingBox;
        Perlin *mPerlin;

        Primitive();
        Primitive(nlohmann::json &json);

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

        /**
         * @brief Return the bounding box for this primitive.
         *
         * @return BoundingBox
         */
        virtual BoundingBox boundingBox() const;

        /**
         * @brief Perform a texture lookup, returning a color.
         */
        void textureLookup(const Vector &intersection, double u, double v, Color &color) const;

        // Ray collision helpers (common to all object types)
        /**
         * @brief Calculate a ray's reflection based on the surface type,
         * set the final ray color, and return the reflection type.
         *
         * @param incoming Incoming ray to be bounced
         * @param intersection Intersection point
         * @param normal Normal vector of the surface at intersection point
         * @return enum Collision
         */
        enum Collision bounce(Ray &incoming, const Vector &intersection, const Vector &normal) const;

        /**
         * @brief Handle a specular reflection.
         *
         * @param incoming Incoming ray to be reflected
         * @param intersection Intersection point
         * @param normal Normal vector of collision surface
         * @return true If the ray should keep bouncing
         * @return false If the ray has been absorbed
         */
        bool specular(Ray &incoming, const Vector &intersection, const Vector &normal) const;

        /**
         * @brief Handle a diffuse reflection.
         *
         * @param incoming Incoming ray to be reflected
         * @param intersection Intersection point
         * @param normal Normal vector of collision surface
         * @return true If the ray should keep bouncing
         * @return false If the ray has been absorbed
         */
        bool diffuse(Ray &incoming, const Vector &intersection, const Vector &normal) const;

        /**
         * @brief Handle a dielectric reflection/refraction.
         *
         * @param incoming Incoming ray to be reflected.
         * @return true If the ray should keep bouncing
         * @return false If the ray has been absorbed
         */
        bool dielectric(Ray &incoming, const Vector &intersection, const Vector &normal, double indexOfRefraction) const;
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
        Vector mTexcoords[3];
        Vector mNormal; // Normal vector, determined by winding order of vertices

        Triangle();
        Triangle(Vector vertices[3], Vector texcoords[3], enum Color::Surface surface, double indexOfRefraction, const Color &color);
        Triangle(nlohmann::json &json);

        enum Collision collide(Ray &incoming, double &t, Color &color) const override;
        BoundingBox boundingBox() const override;

    private:
        void textureLookup(double alpha, double beta, double gamma, const Vector &intersection, Color &color) const;
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

        Sphere();
        Sphere(const Vector &origin, double radius, enum Color::Surface surface, double indexOfRefraction, const Color &color);
        Sphere(nlohmann::json &json);

        virtual enum Collision collide(Ray &incoming, double &t, Color &color) const override;
        virtual BoundingBox boundingBox() const override;

    private:
        void textureLookup(Vector &intersection, Color &color) const;
    };

    class Quad : public Primitive
    {
    public:
        Vector mOrigin;
        Vector mWidth, mHeight;
        Vector mNormal;

        Quad();
        Quad(const Vector &origin, const Vector &width, const Vector &height, enum Color::Surface surface, double indexOfRefraction, const Color &color);
        Quad(nlohmann::json &json);

        enum Collision collide(Ray &incoming, double &t, Color &color) const override;
        BoundingBox boundingBox() const override;

    private:
        void textureLookup(double alpha, double beta, const Vector &intersection, Color &color) const;
        Vector mW; // Used for intersection checking
    };

    class Model : public Primitive
    {
    public:
        ModelMatrix mModelMatrix;

        tinyobj::ObjReader &mObj;

        Model(tinyobj::ObjReader &obj, const Vector &origin, const Vector &front, const Vector &top, const Vector &scale, enum Color::Surface surface, double indexOfRefraction, const Color &color);
        Model(nlohmann::json &json, tinyobj::ObjReader &obj);

        enum Collision collide(Ray &incoming, double &t, Color &color) const override;
        // No texture lookup support
        BoundingBox boundingBox() const override;
    };

    class SphereVolume : public Sphere
    {
    public:
        double mNegInvDensity;

        SphereVolume(const Vector &origin, double radius, double density, Color &color);
        SphereVolume(nlohmann::json &json);

        enum Collision collide(Ray &incoming, double &t, Color &color) const override;
        // No texture lookup support
        BoundingBox boundingBox() const override;
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
        double mLensDiskDiameter;

        Camera();
        Camera(const Vector &origin, const Vector &front, const Vector &top, double focalLength, double emissiveGain);
        Camera(nlohmann::json &json);
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

    // List of textures
    std::vector<std::unique_ptr<STBImage>> mTextures;
    std::vector<std::string> mTextureFilenames;

    Perlin mPerlin;

    Scene();
    ~Scene();

    /**
     * @brief Load the scene data from a JSON file and
     * populate all of the objects at their correct coordinates.
     *
     * @param sceneJsonPath
     */
    void load(std::string sceneJsonPath);

private:
};
