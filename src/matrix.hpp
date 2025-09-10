#pragma once
#include <vector>

typedef std::vector<std::vector<double>> matrix_t;
typedef std::vector<double> vector_t;

#define V_X (0)
#define V_Y (1)
#define V_Z (2)

class Matrix
{
public:
    /**
     * NOTE!
     * All functions in this class return *new* std::vectors. The
     * original vectors are left unmodified. Make sure to take care
     * of any memory issues.
     */

    /**
     * @brief n-dimensional dot product of two vectors
     */
    static double dot(std::vector<double> a, std::vector<double> b);

    /**
     * @brief 3-dimensional cross product of two vectors
     */
    static std::vector<double> cross3(std::vector<double> a, std::vector<double> b);

    /**
     * @brief n-dimensional vector add
     */
    static std::vector<double> vadd(std::vector<double> a, std::vector<double> b);

    /**
     * @brief n-dimensional vector subtract
     */
    static std::vector<double> vsub(std::vector<double> a, std::vector<double> b);

    /**
     * @brief n-dimensional vector multiply with a scalar
     */
    static std::vector<double> vscale(std::vector<double> a, double scalar);

    /**
     * @brief n-dimensional vector norm
     */
    static std::vector<double> vnorm(std::vector<double> a);

    /**
     * @brief m*n matrix add
     */
    static std::vector<std::vector<double>> madd(std::vector<std::vector<double>> a, std::vector<std::vector<double>> b);

    /**
     * @brief m*n matrix subtract
     */
    static std::vector<std::vector<double>> msub(std::vector<std::vector<double>> a, std::vector<std::vector<double>> b);

    /**
     * @brief m*n matrix multiply (remember, order matters!)
     */
    static std::vector<std::vector<double>> mmul(std::vector<std::vector<double>> a, std::vector<std::vector<double>> b);

    /**
     * @brief n-dimensional matrix multiply with a scalar
     */
    static std::vector<std::vector<double>> mscale(std::vector<std::vector<double>> a, double scalar);

    /**
     * @brief m*n matrix transpose
     */
    std::vector<std::vector<double>> transpose(std::vector<std::vector<double>> a);
};