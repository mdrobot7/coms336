#include <fstream>
#include <stdexcept>
#include <string>
#include <iostream>
#include <algorithm>
#include "scene.hpp"

#define TINYOBJLOADER_IMPLEMENTATION // Can only be in ONE cpp file
#include "tiny_obj_loader.h"

namespace object
{
    Primitive::Primitive() {}
    Primitive::Primitive(nlohmann::json json) { (void)json; }

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

    Triangle::Triangle(nlohmann::json json)
    {
        mVertices = matrix_t(3);
        for (int i = 0; i < 3; i++)
        {
            mVertices[i] = vector_t{json["vertices"][i]["x"],
                                    json["vertices"][i]["y"],
                                    json["vertices"][i]["z"]};
        }

        mSpectral = json["spectral"];
        int color = std::stoi((std::string)(json["color"]), 0, 16);
        mColor = intToColor(color);
    }

    Ray Triangle::collide(Ray incoming)
    {
        // TODO
        return incoming;
    }

    Sphere::Sphere(vector_t center, double radius, bool spectral, color_t color)
    {
        mOrigin = center;
        mRadius = radius;
        mSpectral = spectral;
        mColor = color;
    }

    Sphere::Sphere(nlohmann::json json)
    {
        mOrigin = vector_t{json["x"],
                           json["y"],
                           json["z"]};
        mRadius = json["radius"];
        mSpectral = json["spectral"];
        int color = std::stoi((std::string)(json["color"]), 0, 16);
        mColor = intToColor(color);
    }

    Ray Sphere::collide(Ray incoming)
    {
        // TODO
        return incoming;
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
        mColor = intToColor(color);
    }

    Model::Model(tinyobj::ObjReader obj, vector_t origin, vector_t front, vector_t top, vector_t scale) : mObj(obj)
    {
        mOrigin = origin;
        mFront = front;
        mTop = top;
        mScale = scale;
    }

    Model::Model(nlohmann::json json, tinyobj::ObjReader obj) : mObj(obj)
    {
        mOrigin = vector_t{json["origin"]["x"],
                           json["origin"]["y"],
                           json["origin"]["z"]};
        mFront = vector_t{json["front"]["x"],
                          json["front"]["y"],
                          json["front"]["z"]};
        mTop = vector_t{json["top"]["x"],
                        json["top"]["y"],
                        json["top"]["z"]};
        mScale = vector_t{json["scale"]["x"],
                          json["scale"]["y"],
                          json["scale"]["z"]};
    }

    Camera::Camera() {}

    Camera::Camera(vector_t origin, vector_t dir, double focalLength)
    {
        mOrigin = origin;
        mDir = dir;
        mFocalLength = focalLength;
    }

    Camera::Camera(nlohmann::json json)
    {
        mOrigin = vector_t{json["origin"]["x"],
                           json["origin"]["y"],
                           json["origin"]["z"]};
        mDir = vector_t{json["direction"]["x"],
                        json["direction"]["y"],
                        json["direction"]["z"]};
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