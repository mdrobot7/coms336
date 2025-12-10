#include <fstream>
#include <stdexcept>
#include <string>
#include <iostream>
#include <algorithm>
#include "vector.hpp"
#include "scene.hpp"
#include "color.hpp"
#include "common.hpp"

#define TINYOBJLOADER_IMPLEMENTATION // Can only be in ONE cpp file
#include "tiny_obj_loader.h"

namespace object
{
    Primitive::Primitive() {}
    Primitive::Primitive(nlohmann::json json) { (void)json; }

    bool Primitive::specular(Ray &incoming, Vector intersection, Vector normal) const
    {
        static const double fuzziness = 0.0; // TODO: Potentially use later

        incoming.mOrigin = intersection;
        // mDir = mDir - normal * 2 * dot(mDir, normal)
        incoming.mDir.vnorm(Vector::svsub(incoming.mDir, Vector::svscale(normal, 2 * Vector::dot(incoming.mDir, normal))));

        // Add fuzziness. If the fuzzy vector is shot inside the object,
        // the object just absorbs it and the ray stops.
        incoming.mDir = Vector::svadd(incoming.mDir, Vector::svscale(Vector::svrand3(), fuzziness));
        return (Vector::dot(incoming.mDir, normal) > 0.0);
    }

    bool Primitive::diffuse(Ray &incoming, Vector intersection, Vector normal) const
    {
        // Reflect 1 ray with a Lambertian reflection
        incoming.mOrigin = intersection;
        incoming.mDir.vnorm(Vector::svadd(normal, Vector::svrand3()));
        return true;
    }

    bool Primitive::dielectric(Ray &incoming, Vector intersection, Vector normal, double indexOfRefraction) const
    {
        // Check whether we're coming in or out of an object.
        // Normal and incoming vector will be opposite each other.
        bool outsideObject = Vector::dot(incoming.mDir, normal) < 0.0;

        // Determine if we have total internal reflection
        double refractionIndex = outsideObject ? 1.0 / indexOfRefraction : indexOfRefraction;
        double cosTheta = std::fmin(Vector::dot(Vector::svscale(incoming.mDir, -1.0), normal), 1.0);
        double sinTheta = sqrt(1.0 - cosTheta * cosTheta);

        // Schlick's approximation
        double temp = (1.0 - refractionIndex) / (1.0 + refractionIndex);
        temp = temp * temp;
        double schlick = temp + (1.0 - temp) * pow(1.0 - cosTheta, 5.0);
        double random = rand() / ((double)RAND_MAX + 1.0);

        if ((refractionIndex * sinTheta > 1.0) || (schlick > random))
        {
            // No solution for Snell's law, must reflect
            return specular(incoming, intersection, normal);
        }

        // See raytracing in one weekend. Complex math based on Snell's law.
        Vector rOutPerp = Vector::svscale(Vector::svadd(incoming.mDir, Vector::svscale(normal, cosTheta)), refractionIndex);
        Vector rOutParallel = Vector::svscale(normal, -sqrt(fabs(1.0 - Vector::dot(rOutPerp, rOutPerp))));

        incoming.mDir.vnorm(Vector::svadd(rOutPerp, rOutParallel));
        incoming.mOrigin = intersection;
        incoming.mIndexOfRefraction = indexOfRefraction;

        return true;
    }

    Triangle::Triangle(Vector &v0, Vector &v1, Vector &v2, enum Color::Surface surface, double indexOfRefraction, Color &color)
    {
        mVertices[0] = v0;
        mVertices[1] = v1;
        mVertices[2] = v2;
        mSurface = surface;
        mIndexOfRefraction = indexOfRefraction;
        mColor = color;

        // Assuming CCW winding order (standard for OBJ and OpenGL)
        Vector v10, v21 = Vector();
        mNormal.cross3(Vector::svsub(mVertices[1], mVertices[0]), Vector::svsub(mVertices[2], mVertices[1]));
        mNormal.vnorm();
    }

    Triangle::Triangle(nlohmann::json json)
    {
        for (int i = 0; i < 3; i++)
        {
            mVertices[i] = Vector(json["vertices"][i]["x"],
                                  json["vertices"][i]["y"],
                                  json["vertices"][i]["z"]);
        }

        mSurface = Color::stringToSurface(json["surface"]);
        if (mSurface == Color::Surface::DIELECTRIC)
        {
            mIndexOfRefraction = json["indexOfRefraction"];
        }
        int color = std::stoi((std::string)(json["color"]), 0, 16);
        mColor = Color::intToColor(color);

        // Assuming CCW winding order (standard for OBJ and OpenGL)
        mNormal.cross3(Vector::svsub(mVertices[1], mVertices[0]), Vector::svsub(mVertices[2], mVertices[1]));
        mNormal.vnorm();
    }

    enum Primitive::Collision Triangle::collide(Ray &incoming, double &t, Color &color) const
    {
        // Check ray-plane intersection
        double dirDotNorm = Vector::dot(incoming.mDir, mNormal);
        if (CLOSE_TO(dirDotNorm, 0.0))
        {
            // Incoming is parallel
            return Collision::MISSED;
        }

        t = Vector::svsub(mVertices[0], incoming.mOrigin).dot(mNormal) / dirDotNorm;
        if (t < 0)
        {
            // Don't hit things behind us
            return Collision::MISSED;
        }
        else if (CLOSE_TO(t, 0.0))
        {
            // Don't collide with an object we just collided with
            return Collision::MISSED;
        }

        Vector intersection = Vector::svadd(incoming.mOrigin, Vector::svscale(incoming.mDir, t));

        // Split triangle into subtriangles, calculate normals
        Vector normA, normB, normC;
        normA.cross3(Vector::svsub(mVertices[2], mVertices[1]), Vector::svsub(intersection, mVertices[1]));
        normB.cross3(Vector::svsub(mVertices[0], mVertices[2]), Vector::svsub(intersection, mVertices[2]));
        normC.cross3(Vector::svsub(mVertices[1], mVertices[0]), Vector::svsub(intersection, mVertices[0]));

        // Calculate barycentrics
        double alpha = Vector::dot(mNormal, normA) / Vector::dot(mNormal, mNormal);
        double beta = Vector::dot(mNormal, normB) / Vector::dot(mNormal, mNormal);
        double gamma = Vector::dot(mNormal, normC) / Vector::dot(mNormal, mNormal);
        if (alpha < 0.0 || beta < 0.0 || gamma < 0.0)
        {
            // Did not intersect
            return Collision::MISSED;
        }

        // Bounce it
        switch (mSurface)
        {
        case Color::SPECULAR:
            color = Color(1, 1, 1);
            return (specular(incoming, intersection, mNormal) ? Collision::REFLECTED : Collision::ABSORBED);
        case Color::DIFFUSE:
            color = mColor;
            return (diffuse(incoming, intersection, mNormal) ? Collision::REFLECTED : Collision::ABSORBED);
        case Color::DIELECTRIC:
            color = mColor;
            return (dielectric(incoming, intersection, mNormal, mIndexOfRefraction) ? Collision::REFLECTED : Collision::ABSORBED);
        case Color::EMISSIVE:
            color = mColor;
            return Collision::ABSORBED; // Emissive surfaces never reflect
        }
        throw std::invalid_argument("Triangle collision error");
    }

    Sphere::Sphere(Vector &center, double radius, enum Color::Surface surface, double indexOfRefraction, Color &color)
    {
        mOrigin = center;
        mRadius = radius;
        mSurface = surface;
        mIndexOfRefraction = indexOfRefraction;
        mColor = color;
    }

    Sphere::Sphere(nlohmann::json json)
    {
        mOrigin = Vector(json["x"],
                         json["y"],
                         json["z"]);
        mRadius = json["radius"];
        mSurface = Color::stringToSurface(json["surface"]);
        if (mSurface == Color::Surface::DIELECTRIC)
        {
            mIndexOfRefraction = json["indexOfRefraction"];
        }
        int color = std::stoi((std::string)(json["color"]), 0, 16);
        mColor = Color::intToColor(color);
    }

    enum Primitive::Collision Sphere::collide(Ray &incoming, double &t, Color &color) const
    {
        // The math for this is really complicated, it's basically
        // solving a quadratic equation. See Ray Tracing in One Weekend
        Vector centerMinusIncoming = Vector::svsub(mOrigin, incoming.mOrigin);
        double a = Vector::dot(incoming.mDir, incoming.mDir);
        double b = -2.0 * Vector::dot(incoming.mDir, centerMinusIncoming);
        double c = Vector::dot(centerMinusIncoming, centerMinusIncoming) - mRadius * mRadius;
        double discriminant = b * b - 4.0 * a * c;
        if (discriminant < 0)
        {
            // No intersection
            return Collision::MISSED;
        }
        else
        {
            // Take the negative of the +/-, we want the smaller t (closer point)
            t = (-b - sqrt(discriminant)) / (2.0 * a);
            if (mSurface == Color::Surface::DIELECTRIC && CLOSE_TO(fabs(t), 0.0))
            {
                // Dielectrics change ray direction at the surface of
                // the object. That leads to a lot of false Collision::MISSEDs
                // because we're either really close or negative.
                // Grab the further t.
                t = (-b + sqrt(discriminant)) / (2.0 * a);
            }

            if (t < 0)
            {
                // Don't hit things behind us
                return Collision::MISSED;
            }
            else if (CLOSE_TO(t, 0.0))
            {
                // Don't collide with an object we just collided with
                return Collision::MISSED;
            }
        }

        // DEBUG
        if (mSurface == Color::Surface::DIELECTRIC)
        {
            int foo = 0;
            foo++;
        }

        // Bounce it
        Vector intersection = Vector::svadd(incoming.mOrigin, Vector::svscale(incoming.mDir, t));
        Vector normal = Vector::svscale(Vector::svsub(intersection, mOrigin), 1.0 / mRadius);
        switch (mSurface)
        {
        case Color::SPECULAR:
            color = Color{1, 1, 1};
            return (specular(incoming, intersection, normal) ? Collision::REFLECTED : Collision::ABSORBED);
        case Color::DIFFUSE:
            color = mColor;
            return (diffuse(incoming, intersection, normal) ? Collision::REFLECTED : Collision::ABSORBED);
        case Color::DIELECTRIC:
            // color = mColor; // DEBUG
            color = Color{1, 1, 1};
            return (dielectric(incoming, intersection, normal, mIndexOfRefraction) ? Collision::REFLECTED : Collision::ABSORBED);
        case Color::EMISSIVE:
            color = mColor;
            return Collision::ABSORBED; // Emissive surfaces never reflect
        }
        throw std::invalid_argument("Sphere collision error");
    }

    Quad::Quad(Vector &origin, Vector &width, Vector &height, enum Color::Surface surface, double indexOfRefraction, Color &color)
    {
        mOrigin = origin;
        mWidth = width;
        mHeight = height;
        mSurface = surface;
        mIndexOfRefraction = indexOfRefraction;
        mColor = color;

        Vector widthCrossHeight = Vector::scross3(mWidth, mHeight);
        mNormal = Vector::svnorm(widthCrossHeight);
        mW = Vector::svscale(widthCrossHeight, 1.0 / Vector::dot(widthCrossHeight, widthCrossHeight));
    }

    Quad::Quad(nlohmann::json json)
    {
        mOrigin = Vector(json["origin"]["x"],
                         json["origin"]["y"],
                         json["origin"]["z"]);
        mWidth = Vector(json["width"]["x"],
                        json["width"]["y"],
                        json["width"]["z"]);
        mHeight = Vector(json["height"]["x"],
                         json["height"]["y"],
                         json["height"]["z"]);
        mSurface = Color::stringToSurface(json["surface"]);
        if (mSurface == Color::Surface::DIELECTRIC)
        {
            mIndexOfRefraction = json["indexOfRefraction"];
        }
        int color = std::stoi((std::string)(json["color"]), 0, 16);
        mColor = Color::intToColor(color);

        Vector widthCrossHeight = Vector::scross3(mWidth, mHeight);
        mNormal = Vector::svnorm(widthCrossHeight);
        mW = Vector::svscale(widthCrossHeight, 1.0 / Vector::dot(widthCrossHeight, widthCrossHeight));
    }

    enum Primitive::Collision Quad::collide(Ray &incoming, double &t, Color &color) const
    {
        // Check ray-plane intersection
        double dirDotNorm = Vector::dot(incoming.mDir, mNormal);
        if (CLOSE_TO(dirDotNorm, 0.0))
        {
            // Incoming is parallel
            return Collision::MISSED;
        }

        t = Vector::dot(Vector::svsub(mOrigin, incoming.mOrigin), mNormal) / dirDotNorm;
        if (t < 0)
        {
            // Don't hit things behind us
            return Collision::MISSED;
        }
        else if (CLOSE_TO(t, 0.0))
        {
            // Don't collide with an object we just collided with
            return Collision::MISSED;
        }

        Vector intersection = Vector::svadd(incoming.mOrigin, Vector::svscale(incoming.mDir, t));
        Vector planarIntersection = Vector::svsub(intersection, mOrigin);
        double alpha = Vector::dot(mW, Vector::scross3(planarIntersection, mHeight));
        double beta = Vector::dot(mW, Vector::scross3(mWidth, planarIntersection));

        // planarIntersection = alpha * mWidth + beta * mHeight.
        // If alpha and beta are [0.0, 1.0], then the intersection is inside the quad.
        if (!IN_RANGE(alpha, 0.0, 1.0) || !IN_RANGE(beta, 0.0, 1.0))
        {
            return Collision::MISSED;
        }

        // Bounce it
        switch (mSurface)
        {
        case Color::SPECULAR:
            color = Color{1, 1, 1};
            return (specular(incoming, intersection, mNormal) ? Collision::REFLECTED : Collision::ABSORBED);
        case Color::DIFFUSE:
            color = mColor;
            return (diffuse(incoming, intersection, mNormal) ? Collision::REFLECTED : Collision::ABSORBED);
        case Color::DIELECTRIC:
            color = mColor;
            return (dielectric(incoming, intersection, mNormal, mIndexOfRefraction) ? Collision::REFLECTED : Collision::ABSORBED);
        case Color::EMISSIVE:
            color = mColor;
            return Collision::ABSORBED; // Emissive surfaces never reflect
        }
        throw std::invalid_argument("Quad collision error");
    }

    Model::Model(tinyobj::ObjReader obj, Vector &origin, Vector &front, Vector &top, Vector &scale) : mObj(obj)
    {
        mOrigin = origin;
        mFront = Vector::svnorm(front);
        mTop = Vector::svnorm(top);
        mScale = scale;
    }

    Model::Model(nlohmann::json json, tinyobj::ObjReader obj) : mObj(obj)
    {
        mOrigin = Vector(json["origin"]["x"],
                         json["origin"]["y"],
                         json["origin"]["z"]);
        mFront = Vector(json["front"]["x"],
                        json["front"]["y"],
                        json["front"]["z"])
                     .vnorm();
        mTop = Vector(json["top"]["x"],
                      json["top"]["y"],
                      json["top"]["z"])
                   .vnorm();
        mScale = Vector(json["scale"]["x"],
                        json["scale"]["y"],
                        json["scale"]["z"]);
    }

    enum Primitive::Collision Model::collide(Ray &incoming, double &t, Color &color) const
    {
        return Collision::ABSORBED;
    }

    Camera::Camera() {}

    Camera::Camera(Vector &origin, Vector &front, Vector &top, double focalLength)
    {
        mOrigin = origin;
        mFront = front.vnorm();
        mTop = top.vnorm();
        mFocalLength = focalLength;
    }

    Camera::Camera(nlohmann::json json)
    {
        mOrigin = Vector(json["origin"]["x"],
                         json["origin"]["y"],
                         json["origin"]["z"]);
        mFront = Vector(json["front"]["x"],
                        json["front"]["y"],
                        json["front"]["z"])
                     .vnorm();
        mTop = Vector(json["top"]["x"],
                      json["top"]["y"],
                      json["top"]["z"])
                   .vnorm();
        mFocalLength = json["focalLength"];
    }
}

Scene::Scene() {}

void Scene::load(std::string sceneJsonPath)
{
    std::ifstream f(sceneJsonPath);

    using json = nlohmann::json;
    json data = json::parse(f);

    tinyobj::ObjReaderConfig readerConfig;
    readerConfig.mtl_search_path = "./assets/materials"; // Hardcoded, fight me

    // Look at scenes/sample.json for the format
    mCamera = object::Camera(data["camera"]);
    for (json i : data["objects"])
    {
        if (i["type"] == "obj")
        {
            // Attributes and vertices are stored in the tinyobj::ObjReader object.
            // We could spend time deep-copying them out, but that seems foolish.
            // Just keep the reader around.

            // Don't load the same thing multiple times
            size_t fileIndex;
            for (fileIndex = 0; fileIndex < mObjFilenames.size(); fileIndex++)
            {
                if (mObjFilenames[fileIndex] == i["path"])
                {
                    break;
                }
            }
            if (fileIndex != mObjFilenames.size())
            {
                mPrimitives.push_back(std::make_unique<object::Model>(i, mObjReaders[fileIndex]));
            }
            else
            {
                mObjReaders.push_back(tinyobj::ObjReader());
                mObjFilenames.push_back(i["path"]);
                if (!mObjReaders.back().ParseFromFile(i["path"], readerConfig))
                {
                    f.close();
                    throw std::invalid_argument("OBJ file parse failed");
                }
                mPrimitives.push_back(std::make_unique<object::Model>(i, mObjReaders.back()));
            }
        }
        else if (i["type"] == "sphere")
        {
            mPrimitives.push_back(std::make_unique<object::Sphere>(i));
        }
        else if (i["type"] == "triangle")
        {
            mPrimitives.push_back(std::make_unique<object::Triangle>(i));
        }
        else if (i["type"] == "quad")
        {
            mPrimitives.push_back(std::make_unique<object::Quad>(i));
        }
        else
        {
            f.close();
            throw std::invalid_argument("Invalid object in JSON");
        }
    }

    f.close();

    if (mPrimitives.size() == 0)
    {
        throw std::invalid_argument("No objects in the scene");
    }
}