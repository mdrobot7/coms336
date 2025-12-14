#pragma once

#include <vector>
#include "vector.hpp"

class Perlin
{
public:
    Perlin();

    /**
     * @brief Get the Perlin noise value for a particular location.
     * Can be used to scale texture brightness, modify textures,
     * or generate terrain.
     */
    double get(const Vector &vec);

private:
    static constexpr double sFrequency = 100;
    static constexpr int sNumPoints = 256;
    std::vector<Vector> mVectors;
    std::vector<int> mPermX;
    std::vector<int> mPermY;
    std::vector<int> mPermZ;

    /**
     * @brief Perform Perlin interpolation.
     */
    static double interpolate(const Vector c[2][2][2], double u, double v, double w);

    /**
     * @brief Generate a set of permuatation ints.
     */
    static void generatePermutations(std::vector<int> &perms);
};