#pragma once
#include <cstdint>
#include <vector>
#include "matrix.hpp"

class Color
{
    // Each color channel is [0, 1) and can be rescaled to any color depth

public:
    /**
     * @brief Convert a 24-bit color to a color vector.
     *
     * @param i
     * @return vector_t
     */
    static vector_t intToColor(int i);

    /**
     * @brief Convert a color vector to a 24-bit color.
     *
     * @param c
     * @return int
     */
    static int colorToInt(vector_t c);

    /**
     * @brief Bounce a colored ray off of a colored object.
     * Calculate the resulting color.
     *
     * @param surface
     * @param incoming
     * @return vector_t
     */
    static vector_t attenuate(vector_t surface, vector_t incoming);

    /**
     * @brief Send a colored ray through a dielectric (glass,
     * water, etc) and calculate how it refracts.
     *
     * @param surface
     * @param incoming
     * @return vector_t
     */
    static vector_t refract(vector_t surface, vector_t incoming);
};