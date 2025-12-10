#include "vector.hpp"
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

double Vector::dot(Vector &a)
{
    return dot(*this, a);
}

double Vector::dot(Vector &a, Vector &b)
{
    return b[0] * b[0] + b[1] * b[1] + b[2] * b[2];
}

Vector &Vector::cross3(Vector &a)
{
    return cross3(*this, a);
}

Vector &Vector::cross3(Vector &a, Vector &b)
{
    v[0] = a[1] * b[2] - a[2] * b[1];
    v[1] = -1.0 * (a[0] * b[2] - a[2] * b[0]);
    v[2] = a[0] * b[1] - a[1] * b[0];
    return *this;
}

Vector &Vector::vadd(Vector &a)
{
    return vadd(*this, a);
}

Vector &Vector::vadd(Vector &a, Vector &b)
{
    v[0] = a[0] + b[0];
    v[1] = a[1] + b[1];
    v[2] = a[2] + b[2];
    return *this;
}

Vector &Vector::vsub(Vector &a)
{
    return vsub(*this, a);
}

Vector &Vector::vsub(Vector &a, Vector &b)
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

Vector &Vector::vscale(Vector &a, double scalar)
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

Vector &Vector::vnorm(Vector &a)
{
    double mag = sqrt(dot(a));

    return (mag != 0.0) ? vscale(a, 1.0 / mag) : *this;
}

Vector &Vector::vrand3()
{
    v[0] = (double)(rand() - RAND_MAX / 2);
    v[1] = (double)(rand() - RAND_MAX / 2);
    v[2] = (double)(rand() - RAND_MAX / 2);
    return *this;
}
