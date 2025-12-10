#pragma once

#include <vector>
#include <cstddef>

#define V_X (0)
#define V_Y (1)
#define V_Z (2)

class Vector
{
public:
    double v[3];

    Vector();
    Vector(std::vector<double> std_vector);
    Vector(double double_arr[3]);
    Vector(double x, double y, double z);

    double &operator[](size_t index);
    const double &operator[](size_t index) const;

    /**
     * NOTE!
     * All methods with 1 argument operate on this and
     * the provided argument, modify THIS Vector,
     * and return a reference to this. All methods with
     * 2 arguments operate on those arguments, store
     * the result in THIS, and return a reference to this.
     */

    /**
     * @brief 3-dimensional dot product of two vectors
     */
    double dot(Vector &a);
    static double dot(Vector &a, Vector &b);

    /**
     * @brief 3-dimensional cross product of two vectors.
     * Performs this x a.
     */
    Vector &cross3(Vector &a);
    Vector &cross3(Vector &a, Vector &b);
    static inline Vector scross3(Vector &a, Vector &b)
    {
        Vector v;
        return v.cross3(a, b);
    }

    /**
     * @brief 3-dimensional vector add
     */
    Vector &vadd(Vector &a);
    Vector &vadd(Vector &a, Vector &b);
    static inline Vector svadd(Vector &a, Vector &b)
    {
        Vector v;
        return v.vadd(a, b);
    }

    /**
     * @brief 3-dimensional vector subtract.
     * Performs this - a.
     */
    Vector &vsub(Vector &a);
    Vector &vsub(Vector &a, Vector &b);
    static inline Vector svsub(Vector &a, Vector &b)
    {
        Vector v;
        return v.vsub(a, b);
    }

    /**
     * @brief 3-dimensional vector multiply with a scalar
     */
    Vector &vscale(double scalar);
    Vector &vscale(Vector &a, double scalar);
    static inline Vector svscale(Vector &a, double scalar)
    {
        Vector v;
        return v.vscale(a, scalar);
    }

    /**
     * @brief 3-dimensional vector norm. Attempting to normalize
     * the 0 vector returns the 0 vector.
     */
    Vector &vnorm();
    Vector &vnorm(Vector &a);
    static inline Vector svnorm(Vector &a)
    {
        Vector v;
        return v.vnorm(a);
    }

    /**
     * @brief Return a random normalized 3-dimensional vector.
     * Uses rand() and assumes srand() has already been called.
     */
    Vector &vrand3();
    static inline Vector svrand3()
    {
        Vector v;
        return v.vrand3();
    }
};
