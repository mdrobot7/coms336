#include <fstream>
#include <stdexcept>
#include <string>
#include <iostream>
#include <algorithm>
#include "matrix.hpp"
#include "scene.hpp"
#include "color.hpp"
#include "common.hpp"

#define TINYOBJLOADER_IMPLEMENTATION // Can only be in ONE cpp file
#include "tiny_obj_loader.h"

namespace object
{
    Primitive::Primitive() {}
    Primitive::Primitive(nlohmann::json json) { (void)json; }

    bool Primitive::collide(Ray &incoming)
    {
        (void)incoming;
        return false;
    }

    bool Primitive::specular(Ray &incoming, vector_t intersection, vector_t normal)
    {
        static const double fuzziness = 0.0; // TODO: Potentially use later

        incoming.mOrigin = intersection;
        // mDir = mDir - normal * 2 * dot(mDir, normal)
        incoming.mDir = Matrix::vnorm(Matrix::vsub(incoming.mDir, Matrix::vscale(normal, 2 * Matrix::dot(incoming.mDir, normal))));

        // Add fuzziness. If the fuzzy vector is shot inside the object,
        // the object just absorbs it and the ray stops.
        incoming.mDir = Matrix::vadd(incoming.mDir, Matrix::vscale(Matrix::vrand3(), fuzziness));
        return (Matrix::dot(incoming.mDir, normal) < 0.0);
    }

    bool Primitive::diffuse(Ray &incoming, vector_t intersection, vector_t normal)
    {
        // Reflect 1 ray with a Lambertian reflection
        incoming.mOrigin = intersection;
        incoming.mDir = Matrix::vadd(normal, Matrix::vrand3());
        return true;
    }

    bool Primitive::dielectric(Ray &incoming)
    {
        // TODO
        return false;
    }

    Triangle::Triangle(vector_t v0, vector_t v1, vector_t v2, enum Color::Surface surface, color_t color)
    {
        mVertices = matrix_t(3);
        mVertices[0] = v0;
        mVertices[1] = v1;
        mVertices[2] = v2;
        mSurface = surface;
        mColor = color;

        // Assuming CCW winding order (standard for OBJ and OpenGL)
        mNormal = Matrix::cross3(Matrix::vsub(mVertices[1], mVertices[0]), Matrix::vsub(mVertices[2], mVertices[1]));
        mNormal = Matrix::vnorm(mNormal);
    }

    Triangle::Triangle(nlohmann::json json)
    {
        mVertices = matrix_t(3);
        for (int i = 0; i < 3; i++)
        {
            mVertices[i] = vector_t{json["vertices"][i]["x"],
                                    json["vertices"][i]["y"],
                                    json["vertices"][i]["z"]};
        }

        mSurface = Color::stringToSurface(json["surface"]);
        int color = std::stoi((std::string)(json["color"]), 0, 16);
        mColor = Color::intToColor(color);

        // Assuming CCW winding order (standard for OBJ and OpenGL)
        mNormal = Matrix::cross3(Matrix::vsub(mVertices[1], mVertices[0]), Matrix::vsub(mVertices[2], mVertices[1]));
        mNormal = Matrix::vnorm(mNormal);
    }

    bool Triangle::collide(Ray &incoming)
    {
        // Check ray-plane intersection
        double dirDotNorm = Matrix::dot(incoming.mDir, mNormal);
        if (CLOSE_TO(dirDotNorm, 0.0))
        {
            // Incoming is parallel
            return true;
        }

        double t = Matrix::dot(Matrix::vsub(mVertices[0], incoming.mOrigin), mNormal) / dirDotNorm;
        vector_t intersection = Matrix::vadd(incoming.mOrigin, Matrix::vscale(incoming.mDir, t));

        // Split triangle into subtriangles, calculate normals
        vector_t normA = Matrix::cross3(Matrix::vsub(mVertices[2], mVertices[1]), Matrix::vsub(intersection, mVertices[1]));
        vector_t normB = Matrix::cross3(Matrix::vsub(mVertices[0], mVertices[2]), Matrix::vsub(intersection, mVertices[2]));
        vector_t normC = Matrix::cross3(Matrix::vsub(mVertices[1], mVertices[0]), Matrix::vsub(intersection, mVertices[0]));

        // Calculate barycentrics
        double alpha = Matrix::dot(mNormal, normA) / Matrix::dot(mNormal, mNormal);
        double beta = Matrix::dot(mNormal, normB) / Matrix::dot(mNormal, mNormal);
        double gamma = Matrix::dot(mNormal, normC) / Matrix::dot(mNormal, mNormal);
        if (alpha < 0.0 || beta < 0.0 || gamma < 0.0)
        {
            // Did not intersect
            return true;
        }

        // Bounce it
        switch (mSurface)
        {
        case Color::SPECULAR:
            return specular(incoming, intersection, mNormal);
        case Color::DIFFUSE:
            incoming.addCollision(mColor);
            return diffuse(incoming, intersection, mNormal);
        case Color::DIELECTRIC:
            incoming.addCollision(mColor);
            return dielectric(incoming);
        }
        throw std::invalid_argument("Triangle collision error");
    }

    Sphere::Sphere(vector_t center, double radius, enum Color::Surface surface, color_t color)
    {
        mOrigin = center;
        mRadius = radius;
        mSurface = surface;
        mColor = color;
    }

    Sphere::Sphere(nlohmann::json json)
    {
        mOrigin = vector_t{json["x"],
                           json["y"],
                           json["z"]};
        mRadius = json["radius"];
        mSurface = Color::stringToSurface(json["surface"]);
        int color = std::stoi((std::string)(json["color"]), 0, 16);
        mColor = Color::intToColor(color);
    }

    bool Sphere::collide(Ray &incoming)
    {
        // The math for this is really complicated, it's basically
        // solving a quadratic equation. See Ray Tracing in One Weekend
        vector_t centerMinusIncoming = Matrix::vsub(mOrigin, incoming.mOrigin);
        double a = Matrix::dot(incoming.mDir, incoming.mDir);
        double b = -2.0 * Matrix::dot(incoming.mDir, centerMinusIncoming);
        double c = Matrix::dot(centerMinusIncoming, centerMinusIncoming) - mRadius * mRadius;
        double discriminant = b * b - 4.0 * a * c;
        double t = 0;
        if (discriminant < 0)
        {
            // No intersection
            return false;
        }
        else
        {
            // Take the negative of the +/-, we want the smaller t (closer point)
            t = (-b - sqrt(discriminant)) / (2.0 * a);
            if (t < 0)
            {
                // Don't hit things behind us
                return false;
            }
        }

        // Bounce it
        vector_t intersection = Matrix::vadd(incoming.mOrigin, Matrix::vscale(incoming.mDir, t));
        vector_t normal = Matrix::vscale(Matrix::vsub(intersection, mOrigin), 1.0 / mRadius);
        switch (mSurface)
        {
        case Color::SPECULAR:
            return specular(incoming, intersection, normal);
        case Color::DIFFUSE:
            incoming.addCollision(mColor);
            return diffuse(incoming, intersection, normal);
        case Color::DIELECTRIC:
            incoming.addCollision(mColor);
            return dielectric(incoming);
        }
        throw std::invalid_argument("Sphere collision error");
    }

    Light::Light(vector_t center, color_t color)
    {
        mOrigin = center;
        mColor = color;
    }

    Light::Light(nlohmann::json json)
    {
        mOrigin = vector_t{json["x"],
                           json["y"],
                           json["z"]};
        int color = std::stoi(std::string(json["color"]), 0, 16);
        mColor = Color::intToColor(color);
    }

    Model::Model(tinyobj::ObjReader obj, vector_t origin, vector_t front, vector_t top, vector_t scale) : mObj(obj)
    {
        mOrigin = origin;
        mFront = Matrix::vnorm(front);
        mTop = Matrix::vnorm(top);
        mScale = scale;
    }

    Model::Model(nlohmann::json json, tinyobj::ObjReader obj) : mObj(obj)
    {
        mOrigin = vector_t{json["origin"]["x"],
                           json["origin"]["y"],
                           json["origin"]["z"]};
        mFront = Matrix::vnorm(vector_t{json["front"]["x"],
                                        json["front"]["y"],
                                        json["front"]["z"]});
        mTop = Matrix::vnorm(vector_t{json["top"]["x"],
                                      json["top"]["y"],
                                      json["top"]["z"]});
        mScale = vector_t{json["scale"]["x"],
                          json["scale"]["y"],
                          json["scale"]["z"]};
    }

    Camera::Camera() {}

    Camera::Camera(vector_t origin, vector_t front, vector_t top, double focalLength)
    {
        mOrigin = origin;
        mFront = Matrix::vnorm(front);
        mTop = Matrix::vnorm(top);
        mFocalLength = focalLength;
    }

    Camera::Camera(nlohmann::json json)
    {
        mOrigin = vector_t{json["origin"]["x"],
                           json["origin"]["y"],
                           json["origin"]["z"]};
        mFront = Matrix::vnorm(vector_t{json["front"]["x"],
                                        json["front"]["y"],
                                        json["front"]["z"]});

        mTop = Matrix::vnorm(vector_t{json["top"]["x"],
                                      json["top"]["y"],
                                      json["top"]["z"]});
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
                mPrimitives.push_back(object::Model(i, mObjReaders[fileIndex]));
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
                mPrimitives.push_back(object::Model(i, mObjReaders.back()));
            }
        }
        else if (i["type"] == "light")
        {
            mLights.push_back(object::Light(i));
        }
        else if (i["type"] == "sphere")
        {
            mPrimitives.push_back(object::Sphere(i));
        }
        else if (i["type"] == "triangle")
        {
            mPrimitives.push_back(object::Triangle(i));
        }
        else
        {
            f.close();
            throw std::invalid_argument("Invalid object in JSON");
        }
    }

    f.close();
}