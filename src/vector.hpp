#pragma once

#include <vector>

#define V_X (0)
#define V_Y (1)
#define V_Z (2)

class Vector
{
public:
    double v[3];

    Vector(std::vector<double> std_vector);
    Vector(double double_arr[3]);
    Vector(double x, double y, double z);

    double &operator[](size_t index);

    /**
     * NOTE!
     * All methods that return vectors modify THIS Vector
     * and return a reference to this.
     */

    /**
     * @brief 3-dimensional dot product of two vectors
     */
    double dot(Vector &a);

    /**
     * @brief 3-dimensional cross product of two vectors
     */
    const Vector &cross3(Vector &a);

    /**
     * @brief 3-dimensional vector add
     */
    const Vector &vadd(Vector &a);

    /**
     * @brief 3-dimensional vector subtract
     */
    const Vector &vsub(Vector &a);

    /**
     * @brief 3-dimensional vector multiply with a scalar
     */
    const Vector &vscale(double scalar);

    /**
     * @brief 3-dimensional vector norm. Attempting to normalize
     * the 0 vector returns the 0 vector.
     */
    const Vector &vnorm();

    /**
     * @brief Return a random normalized 3-dimensional vector.
     * Uses rand() and assumes srand() has already been called.
     */
    const Vector &vrand3();
};
