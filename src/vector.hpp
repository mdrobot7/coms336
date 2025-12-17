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
    double dot(const Vector &a);
    static double dot(const Vector &a, const Vector &b);

    /**
     * @brief 3-dimensional cross product of two vectors.
     * Performs this x a.
     */
    Vector &cross3(const Vector &a);
    Vector &cross3(const Vector &a, const Vector &b);
    static inline Vector scross3(const Vector &a, const Vector &b)
    {
        Vector v;
        return v.cross3(a, b);
    }

    /**
     * @brief 3-dimensional vector add
     */
    Vector &vadd(const Vector &a);
    Vector &vadd(const Vector &a, const Vector &b);
    static inline Vector svadd(const Vector &a, const Vector &b)
    {
        Vector v;
        return v.vadd(a, b);
    }

    /**
     * @brief 3-dimensional vector subtract.
     * Performs this - a.
     */
    Vector &vsub(const Vector &a);
    Vector &vsub(const Vector &a, const Vector &b);
    static inline Vector svsub(const Vector &a, const Vector &b)
    {
        Vector v;
        return v.vsub(a, b);
    }

    /**
     * @brief 3-dimensional vector multiply with a scalar
     */
    Vector &vscale(double scalar);
    Vector &vscale(const Vector &a, double scalar);
    static inline Vector svscale(const Vector &a, double scalar)
    {
        Vector v;
        return v.vscale(a, scalar);
    }

    /**
     * @brief 3-dimensional vector norm. Attempting to normalize
     * the 0 vector returns the 0 vector.
     */
    Vector &vnorm();
    Vector &vnorm(const Vector &a);
    static inline Vector svnorm(const Vector &a)
    {
        Vector v;
        return v.vnorm(a);
    }

    /**
     * @brief Clamp all elements of a vector in the range [0, clip]
     * (inclusive).
     */
    Vector &vclip(double clip);
    Vector &vclip(const Vector &a, double clip);
    static inline Vector svclip(const Vector &a, double clip)
    {
        Vector v;
        return v.vclip(a, clip);
    }

    /**
     * @brief Return a random normalized 3-dimensional vector.
     * Uses thread-safe C++ random number generation.
     */
    Vector &vrand3();
    static inline Vector svrand3()
    {
        Vector v;
        return v.vrand3();
    }

    /**
     * @brief Returns true if all three of a vector's dimensions
     * are close to 0. False otherwise.
     */
    bool closeToZero() const;
    static inline bool closeToZero(const Vector &a)
    {
        return a.closeToZero();
    }
};

class ModelMatrix
{
public:
    Vector mOrigin;
    Vector mFront; // Model's +Z axis
    Vector mTop;   // Model's +Y axis
    Vector mRight; // Model's +X axis
    Vector mScale;

    ModelMatrix();
    ModelMatrix(const Vector &origin, const Vector &front, const Vector &top, const Vector &scale);

    /**
     * @brief Turn a 3-vector into a homogeneous 4-vector,
     * multiply by the model matrix, convert back to a
     * 3-vector and return.
     *
     * Modifies vec3 and returns a reference to vec3.
     */
    Vector &mul(Vector &vec3) const;
};
