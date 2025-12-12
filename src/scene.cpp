#include <fstream>
#include <stdexcept>
#include <string>
#include <iostream>
#include <algorithm>
#include "vector.hpp"
#include "scene.hpp"
#include "color.hpp"
#include "common.hpp"

namespace object
{
    Primitive::Primitive() {}
    Primitive::Primitive(nlohmann::json &json) { (void)json; }

    enum Primitive::Collision Primitive::bounce(Ray &incoming, const Vector &intersection, const Vector &normal, Color &color) const
    {
        switch (mSurface)
        {
        case Color::SPECULAR:
            return (specular(incoming, intersection, normal) ? Collision::REFLECTED : Collision::ABSORBED);
        case Color::DIFFUSE:
            return (diffuse(incoming, intersection, normal) ? Collision::REFLECTED : Collision::ABSORBED);
        case Color::DIELECTRIC:
            return (dielectric(incoming, intersection, normal, mIndexOfRefraction) ? Collision::REFLECTED : Collision::ABSORBED);
        case Color::EMISSIVE:
            return Collision::ABSORBED; // Emissive surfaces never reflect
        }
        throw std::invalid_argument("Collision error");
    }

    bool Primitive::specular(Ray &incoming, const Vector &intersection, const Vector &normal) const
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

    bool Primitive::diffuse(Ray &incoming, const Vector &intersection, const Vector &normal) const
    {
        // Reflect 1 ray with a Lambertian reflection
        incoming.mOrigin = intersection;
        incoming.mDir.vnorm(Vector::svadd(normal, Vector::svrand3()));
        return true;
    }

    bool Primitive::dielectric(Ray &incoming, const Vector &intersection, const Vector &normal, double indexOfRefraction) const
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

    Triangle::Triangle() {}

    Triangle::Triangle(const Vector &v0, const Vector &v1, const Vector &v2, enum Color::Surface surface, double indexOfRefraction, const Color &color)
    {
        mVertices[0] = v0;
        mVertices[1] = v1;
        mVertices[2] = v2;
        mSurface = surface;
        mIndexOfRefraction = indexOfRefraction;
        mColor = color;

        // Assuming CCW winding order (standard for OBJ and OpenGL)
        mNormal.cross3(Vector::svsub(mVertices[1], mVertices[0]), Vector::svsub(mVertices[2], mVertices[1]));
        mNormal.vnorm();
    }

    Triangle::Triangle(nlohmann::json &json)
    {
        for (int i = 0; i < 3; i++)
        {
            mVertices[i] = Vector(json["vertices"][i]["x"],
                                  json["vertices"][i]["y"],
                                  json["vertices"][i]["z"]);
        }

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

        if (mSurface == Color::SPECULAR || mSurface == Color::DIELECTRIC)
        {
            color = Color(1, 1, 1);
        }
        else
        {
            textureLookup(alpha, beta, gamma, color);
        }

        // Bounce it
        return bounce(incoming, intersection, mNormal, color);
    }

    void Triangle::textureLookup(double alpha, double beta, double gamma, Color &color) const
    {
        if (!mTexture)
        {
            color = mColor;
            return;
        }

        // Thanks stack overflow https://stackoverflow.com/questions/17164376/inferring-u-v-for-a-point-in-a-triangle-from-vertex-u-vs
        double u = alpha * mTexcoords[0][0] + beta * mTexcoords[1][0] + gamma * mTexcoords[2][0];
        double v = alpha * mTexcoords[0][1] + beta * mTexcoords[1][1] + gamma * mTexcoords[2][1];

        color = mTexture->getUv(u, v);
    }

    Sphere::Sphere() {}

    Sphere::Sphere(const Vector &center, double radius, enum Color::Surface surface, double indexOfRefraction, const Color &color)
    {
        mOrigin = center;
        mRadius = radius;
        mSurface = surface;
        mIndexOfRefraction = indexOfRefraction;
        mColor = color;
    }

    Sphere::Sphere(nlohmann::json &json)
    {
        mOrigin = Vector(json["x"],
                         json["y"],
                         json["z"]);
        mRadius = json["radius"];
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

        Vector intersection = Vector::svadd(incoming.mOrigin, Vector::svscale(incoming.mDir, t));
        Vector normal = Vector::svscale(Vector::svsub(intersection, mOrigin), 1.0 / mRadius);
        if (mSurface == Color::SPECULAR || mSurface == Color::DIELECTRIC)
        {
            color = Color(1, 1, 1);
        }
        else
        {
            textureLookup(intersection, color);
        }

        // Bounce it
        return bounce(incoming, intersection, normal, color);
    }

    void Sphere::textureLookup(Vector &intersection, Color &color) const
    {
        if (!mTexture)
        {
            color = mColor;
            return;
        }

        // Convert intersection point to spherical coordinates
        double phi = atan2(-mRadius * intersection[V_Z], intersection[V_X]) + M_PI;
        double theta = acos(-intersection[V_Y]);

        double u = phi / (2.0 * M_PI);
        double v = theta / M_PI;
        color = mTexture->getUv(u, v);
    }

    Quad::Quad() {}

    Quad::Quad(const Vector &origin, const Vector &width, const Vector &height, enum Color::Surface surface, double indexOfRefraction, const Color &color)
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

    Quad::Quad(nlohmann::json &json)
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

        if (mSurface == Color::SPECULAR || mSurface == Color::DIELECTRIC)
        {
            color = Color(1, 1, 1);
        }
        else
        {
            textureLookup(alpha, beta, color);
        }

        // Bounce it
        return bounce(incoming, intersection, mNormal, color);
    }

    void Quad::textureLookup(double alpha, double beta, Color &color) const
    {
        // Intersection testing gives us alpha and beta, which are
        // the same as u and v.
        color = mTexture->getUv(alpha, beta);
    }

    Model::Model(tinyobj::ObjReader &obj, const Vector &origin, const Vector &front, const Vector &top, const Vector &scale, enum Color::Surface surface, double indexOfRefraction, const Color &color) : mObj(obj)
    {
        mModelMatrix = ModelMatrix(origin, Vector::svnorm(front), Vector::svnorm(top), scale);
        mSurface = surface;
        mIndexOfRefraction = indexOfRefraction;
        mColor = color;
    }

    Model::Model(nlohmann::json &json, tinyobj::ObjReader &obj) : mObj(obj)
    {
        mModelMatrix = ModelMatrix(
            Vector(json["origin"]["x"],
                   json["origin"]["y"],
                   json["origin"]["z"]),
            Vector(json["front"]["x"],
                   json["front"]["y"],
                   json["front"]["z"])
                .vnorm(),
            Vector(json["top"]["x"],
                   json["top"]["y"],
                   json["top"]["z"])
                .vnorm(),
            Vector(json["scale"]["x"],
                   json["scale"]["y"],
                   json["scale"]["z"]));
    }

    enum Primitive::Collision Model::collide(Ray &incoming, double &t, Color &color) const
    {
        Triangle tri = Triangle(Vector(), Vector(), Vector(), mSurface, mIndexOfRefraction, mColor);
        Ray closestRay;
        t = std::numeric_limits<double>::infinity();
        Collision closestCollision = Collision::MISSED;
        Ray thisRay = Ray(incoming);
        double thisT;
        Color thisColor;
        Collision thisCollision;

        // Essentially rewrite the main render loop but for only this model
        auto &attrib = mObj.GetAttrib();
        auto &shapes = mObj.GetShapes();
        auto &materials = mObj.GetMaterials();
        int indexOffset = 0;
        for (size_t s = 0; s < shapes.size(); s++)
        {
            const tinyobj::shape_t &shape = shapes[s];
            size_t numTriangles = shape.mesh.num_face_vertices.size();
            for (size_t n = 0; n < numTriangles; n++)
            {
                // Every face is going to be 3 vertices (almost always)
                // Fill in our triangle
                for (int i = 0; i < 3; i++)
                {
                    // Index buffer lookup
                    tinyobj::index_t index = shape.mesh.indices[indexOffset + i];
                    // Vertex buffer lookup
                    for (int j = 0; j < 3; j++)
                    {
                        tri.mVertices[i].v[j] = attrib.vertices[3 * size_t(index.vertex_index) + j];
                    }

                    // Handle scaling, rotation, and positioning (model matrix).
                    // Do on the fly so we can do proper object instancing (vertex shader-style)
                    mModelMatrix.mul(tri.mVertices[i]);
                }
                // Fill in surface normal assuming CCW winding order (standard for OBJ and OpenGL)
                tri.mNormal.cross3(Vector::svsub(tri.mVertices[1], tri.mVertices[0]), Vector::svsub(tri.mVertices[2], tri.mVertices[1]));
                tri.mNormal.vnorm();

                thisCollision = tri.collide(thisRay, thisT, thisColor);
                if (thisCollision != Collision::MISSED && thisT < t)
                {
                    // Found a closer collision
                    closestRay = Ray(thisRay);
                    t = thisT;
                    color = Color(thisColor);
                    closestCollision = thisCollision;
                }
                indexOffset += 3;
            }
        }

        incoming = Ray(closestRay);
        // t, color set during execution
        return closestCollision;
    }

    Camera::Camera() {}

    Camera::Camera(const Vector &origin, const Vector &front, const Vector &top, double focalLength)
    {
        mOrigin = origin;
        mFront = Vector::svnorm(front);
        mTop = Vector::svnorm(top);
        mFocalLength = focalLength;
    }

    Camera::Camera(nlohmann::json &json)
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

        // Fill in common attributes
        auto &p = mPrimitives.back();
        p->mSurface = Color::stringToSurface(i["surface"]);
        if (p->mSurface == Color::Surface::DIELECTRIC)
        {
            p->mIndexOfRefraction = i["indexOfRefraction"];
        }
        try
        {
            int color = std::stoi((std::string)(i["texture"]), 0, 16);
            p->mColor = Color::intToColor(color);
            p->mTexture = NULL;
        }
        catch (std::invalid_argument const &)
        {
            // Texture is a path instead of a color
            size_t fileIndex;
            for (fileIndex = 0; fileIndex < mTextureFilenames.size(); fileIndex++)
            {
                if (mTextureFilenames[fileIndex] == i["texture"])
                {
                    break;
                }
            }
            if (fileIndex == mTextureFilenames.size())
            {
                mTextures.push_back(STBImage(i["texture"]));
                mTextureFilenames.push_back(i["texture"]);
            }
            p->mTexture = &mTextures[fileIndex];
        }
    }

    f.close();

    if (mPrimitives.size() == 0)
    {
        throw std::invalid_argument("No objects in the scene");
    }
}