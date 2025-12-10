#include "vector.hpp"
#include <stdexcept>
#include <cmath>
#include <cstdlib>

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

double Vector::dot(Vector &a)
{
    return v[0] * a[0] + v[1] * a[1] + v[2] * a[2];
}

const Vector &Vector::cross3(Vector &a)
{
    v[0] = a[1] * v[2] - a[2] * v[1];
    v[1] = -1.0 * (a[0] * v[2] - a[2] * v[0]);
    v[2] = a[0] * v[1] - a[1] * v[0];
    return *this;
}

const Vector &Vector::vadd(Vector &a)
{
    v[0] += a[0];
    v[1] += a[1];
    v[2] += a[2];
    return *this;
}

const Vector &Vector::vsub(Vector &a)
{
    v[0] -= a[0];
    v[1] -= a[1];
    v[2] -= a[2];
    return *this;
}

const Vector &Vector::vscale(double scalar)
{
    v[0] *= scalar;
    v[1] *= scalar;
    v[2] *= scalar;
    return *this;
}

const Vector &Vector::vnorm()
{
    double mag = sqrt(dot(*this));

    return (mag != 0.0) ? vscale(1.0 / mag) : *this;
}

const Vector &Vector::vrand3()
{
    v[0] = (double)(rand() - RAND_MAX / 2);
    v[1] = (double)(rand() - RAND_MAX / 2);
    v[2] = (double)(rand() - RAND_MAX / 2);
    return *this;
}
