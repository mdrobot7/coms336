#include "matrix.hpp"
#include <vector>
#include <stdexcept>
#include <cmath>

double Matrix::dot(std::vector<double> a, std::vector<double> b)
{
    if (a.size() != b.size())
    {
        throw std::invalid_argument("Vectors are different sizes");
    }

    double ret = 0.0;
    for (size_t i = 0; i < a.size(); i++)
    {
        ret += a[i] * b[i];
    }

    return ret;
}

std::vector<double> Matrix::cross3(std::vector<double> a, std::vector<double> b)
{
    if (a.size() != 3 || b.size() != 3)
    {
        throw std::invalid_argument("Vectors are not size 3");
    }

    std::vector<double> ret(3);
    ret[0] = a[1] * b[2] - a[2] * b[1];
    ret[1] = -1 * (a[0] * b[2] - a[2] * b[0]);
    ret[2] = a[0] * b[1] - a[1] * b[0];
    return ret;
}

std::vector<double> Matrix::vadd(std::vector<double> a, std::vector<double> b)
{
    if (a.size() != b.size())
    {
        throw std::invalid_argument("Vectors are different sizes");
    }

    std::vector<double> ret(a.size());
    for (size_t i = 0; i < a.size(); i++)
    {
        ret[i] = a[i] + b[i];
    }
    return ret;
}

std::vector<double> Matrix::vsub(std::vector<double> a, std::vector<double> b)
{
    return vadd(a, vscale(b, -1.0));
}

std::vector<double> Matrix::vscale(std::vector<double> a, double scalar)
{
    std::vector<double> ret(a.size());
    for (size_t i = 0; i < a.size(); i++)
    {
        ret[i] = a[i] * scalar;
    }
    return ret;
}

std::vector<double> Matrix::vnorm(std::vector<double> a)
{
    std::vector<double> ret(a.size());

    double mag = 0.0;
    for (double i : a)
    {
        mag += i * i;
    }
    mag = sqrt(mag);

    return vscale(a, 1.0 / mag);
}

std::vector<std::vector<double>> Matrix::madd(std::vector<std::vector<double>> a, std::vector<std::vector<double>> b)
{
    if (a.size() != b.size() || a[0].size() != b[0].size())
    {
        throw std::invalid_argument("Matrices are different sizes");
    }

    size_t rows = a.size();
    size_t cols = a[0].size();
    std::vector<std::vector<double>> ret(rows);
    for (size_t i = 0; i < rows; i++)
    {
        ret[i] = std::vector<double>(cols);
        for (size_t j = 0; j < cols; i++)
        {
            ret[i][j] = a[i][j] + b[i][j];
        }
    }
    return ret;
}

std::vector<std::vector<double>> Matrix::msub(std::vector<std::vector<double>> a, std::vector<std::vector<double>> b)
{
    return madd(a, mscale(b, -1.0));
}

std::vector<std::vector<double>> Matrix::mmul(std::vector<std::vector<double>> a, std::vector<std::vector<double>> b)
{
    // [a*b][c*d] = [a*d], b must equal c
    if (a[0].size() != b.size())
    {
        throw std::invalid_argument("Matrices are not multiplicable");
    }

    size_t rows = a.size();
    size_t cols = b[0].size();
    std::vector<std::vector<double>> ret(rows);
    for (size_t i = 0; i < rows; i++)
    {
        ret[i] = std::vector<double>(cols);
        for (size_t j = 0; j < cols; j++)
        {
            // Fill in ret[i, j] by taking the dot product of
            // row i of matrix A and col j of matrix B.
            for (size_t x = 0; x < a[0].size(); x++)
            {
                ret[i][j] += a[i][x] * b[x][j];
            }
        }
    }
    return ret;
}

std::vector<std::vector<double>> Matrix::mscale(std::vector<std::vector<double>> a, double scalar)
{
    std::vector<std::vector<double>> ret(a.size());
    for (size_t i = 0; i < a.size(); i++)
    {
        ret[i] = std::vector<double>(a[0].size());
        for (size_t j = 0; j < a[0].size(); j++)
        {
            ret[i][j] = a[i][j] * scalar;
        }
    }
    return ret;
}

std::vector<std::vector<double>> Matrix::transpose(std::vector<std::vector<double>> a)
{
    std::vector<std::vector<double>> ret(a[0].size());

    for (size_t i = 0; i < a[0].size(); i++)
    {
        ret[i] = std::vector<double>(a[0].size());
        for (size_t j = 0; j < a.size(); j++)
        {
            ret[i][j] = a[j][i];
        }
    }
    return ret;
}
