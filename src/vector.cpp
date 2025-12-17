#include "vector.hpp"
#include "common.hpp"
#include <stdexcept>
#include <cmath>
#include <cstdlib>

Vector::Vector() {}

Vector::Vector(std::vector<double> std_vector)
{
    if (std_vector.size() != 3)
    {
        throw std::invalid_argument("Vector not of size 3.");
    }
    v[0] = std_vector[0];
    v[1] = std_vector[1];
    v[2] = std_vector[2];
}

Vector::Vector(double double_arr[3])
{
    v[0] = double_arr[0];
    v[1] = double_arr[1];
    v[2] = double_arr[2];
}

Vector::Vector(double x, double y, double z)
{
    v[0] = x;
    v[1] = y;
    v[2] = z;
}

double &Vector::operator[](size_t index)
{
    return v[index];
}

const double &Vector::operator[](size_t index) const
{
    return v[index];
}

double Vector::dot(const Vector &a)
{
    return dot(*this, a);
}

double Vector::dot(const Vector &a, const Vector &b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

Vector &Vector::cross3(const Vector &a)
{
    return cross3(*this, a);
}

Vector &Vector::cross3(const Vector &a, const Vector &b)
{
    v[0] = a[1] * b[2] - a[2] * b[1];
    v[1] = -1.0 * (a[0] * b[2] - a[2] * b[0]);
    v[2] = a[0] * b[1] - a[1] * b[0];
    return *this;
}

Vector &Vector::vadd(const Vector &a)
{
    return vadd(*this, a);
}

Vector &Vector::vadd(const Vector &a, const Vector &b)
{
    v[0] = a[0] + b[0];
    v[1] = a[1] + b[1];
    v[2] = a[2] + b[2];
    return *this;
}

Vector &Vector::vsub(const Vector &a)
{
    return vsub(*this, a);
}

Vector &Vector::vsub(const Vector &a, const Vector &b)
{
    v[0] = a[0] - b[0];
    v[1] = a[1] - b[1];
    v[2] = a[2] - b[2];
    return *this;
}

Vector &Vector::vscale(double scalar)
{
    return vscale(*this, scalar);
}

Vector &Vector::vscale(const Vector &a, double scalar)
{
    v[0] = a[0] * scalar;
    v[1] = a[1] * scalar;
    v[2] = a[2] * scalar;
    return *this;
}

Vector &Vector::vnorm()
{
    return vnorm(*this);
}

Vector &Vector::vnorm(const Vector &a)
{
    double mag = sqrt(dot(a, a));

    return (mag != 0.0) ? vscale(a, 1.0 / mag) : *this;
}

Vector &Vector::vclip(double clip)
{
    return vclip(*this, clip);
}

Vector &Vector::vclip(const Vector &a, double clip)
{
    v[0] = CLAMP(a[0], 0, clip);
    v[1] = CLAMP(a[1], 0, clip);
    v[2] = CLAMP(a[2], 0, clip);
    return *this;
}

Vector &Vector::vrand3()
{
    v[0] = randDist(randGen);
    v[1] = randDist(randGen);
    v[2] = randDist(randGen);
    return this->vnorm();
}

bool Vector::closeToZero() const
{
    return CLOSE_TO(v[0], 0.0) && CLOSE_TO(v[1], 0.0) && CLOSE_TO(v[2], 0.0);
}

ModelMatrix::ModelMatrix() {}

ModelMatrix::ModelMatrix(const Vector &origin, const Vector &front, const Vector &top, const Vector &scale)
{
    mOrigin = origin;
    mFront = front;
    mTop = top;
    mRight = Vector::scross3(Vector::svscale(mFront, -1.0), mTop);
    mScale = scale;
}

Vector &ModelMatrix::mul(Vector &vec3) const
{
    /*
        Essentially this amounts to a change of basis,
        translation, and scale. Doing it with discrete operations
        instead of a single homogeneous matrix because
        it's easier to write and performance is probably
        close enough.

        Little hack  for change of basis because top and
        front are guaranteed to be orthogonal:
        V_i = (V dot b_i) / |b_i|^2
    */
    Vector temp;

    // Handle rotation with change of basis
    temp[0] = Vector::dot(vec3, mRight) / Vector::dot(mRight, mRight);
    temp[1] = Vector::dot(vec3, mTop) / Vector::dot(mTop, mTop);
    temp[2] = Vector::dot(vec3, mFront) / Vector::dot(mFront, mFront);

    // Handle scale
    temp[0] *= mScale[0];
    temp[1] *= mScale[1];
    temp[2] *= mScale[2];

    // Handle translation
    temp[0] += mOrigin[0];
    temp[1] += mOrigin[1];
    temp[2] += mOrigin[2];

    vec3 = temp;
    return vec3;
}
