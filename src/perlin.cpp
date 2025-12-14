#include "perlin.hpp"

#include <random>
#include <cstdlib>
#include <algorithm>
#include "common.hpp"

Perlin::Perlin()
{
    // Generate 4 random blocks: one random vector block
    // and 3 "modifiers" (permutations).
    mVectors = std::vector<Vector>(sNumPoints);
    std::generate(mVectors.begin(), mVectors.end(), Vector::svrand3);

    generatePermutations(mPermX);
    generatePermutations(mPermY);
    generatePermutations(mPermZ);
}

double Perlin::get(const Vector &vec)
{
    // Increase frequency
    Vector sVec = Vector::svscale(vec, sFrequency);

    // Trilinear interpolation with random vectors
    double u = sVec[V_X] - std::floor(sVec[V_X]);
    double v = sVec[V_Y] - std::floor(sVec[V_Y]);
    double w = sVec[V_Z] - std::floor(sVec[V_Z]);

    int i = (int)(sVec[V_X]);
    int j = (int)(sVec[V_Y]);
    int k = (int)(sVec[V_Z]);
    Vector c[2][2][2];

    for (int di = 0; di < 2; di++)
    {
        for (int dj = 0; dj < 2; dj++)
        {
            for (int dk = 0; dk < 2; dk++)
            {
                c[di][dj][dk] = mVectors[mPermX[(i + di) & 255] ^
                                         mPermY[(j + dj) & 255] ^
                                         mPermZ[(k + dk) & 255]];
            }
        }
    }

    return interpolate(c, u, v, w);
}

double Perlin::interpolate(const Vector c[2][2][2], double u, double v, double w)
{
    // Hermitian smoothing
    double uu = u * u * (3 - 2 * u);
    double vv = v * v * (3 - 2 * v);
    double ww = w * w * (3 - 2 * w);
    double accum = 0.0;

    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            for (int k = 0; k < 2; k++)
            {
                Vector weight(u - i, v - j, w - k);
                double term1 = (i * uu + (1 - i) * (1 - uu));
                double term2 = (j * vv + (1 - j) * (1 - vv));
                double term3 = (k * ww + (1 - k) * (1 - ww));
                accum += term1 * term2 * term3 * Vector::dot(c[i][j][k], weight);
            }
        }
    }

    return accum;
}

void Perlin::generatePermutations(std::vector<int> &perms)
{
    int count = 0;
    std::generate(perms.begin(), perms.end(), [&]()
                  { return ++count; });
    auto random = std::default_random_engine{};
    std::shuffle(perms.begin(), perms.end(), random);
}
