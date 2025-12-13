#pragma once

#include <vector>
#include "ray.hpp"
#include "boundingBox.hpp"
#include "scene.hpp"

class BoundingVolumeHierarchy
{
public:
    object::Primitive *mPrimitive;
    BoundingBox mBbox;

    BoundingVolumeHierarchy();
    BoundingVolumeHierarchy(std::vector<object::Primitive> &primitives);
    BoundingVolumeHierarchy(std::vector<object::Primitive> &primitives, size_t start, size_t end);

    /**
     * @brief Checks if a ray hits a bounding volume. If so,
     * return the Primitive it contains, otherwise NULL.
     *
     * If the BVH is traversed successfully this should return the
     * primitive that was hit.
     *
     * If any level of the BVH results in misses for both the
     * left and right child the method returns NULL.
     */
    object::Primitive *intersects(Ray &r);

private:
    BoundingVolumeHierarchy *mLeft;
    BoundingVolumeHierarchy *mRight;

    void destroySubtree(BoundingVolumeHierarchy *subtree);
};
