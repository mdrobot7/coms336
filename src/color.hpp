#pragma once
#include <cstdint>
#include <vector>
#include <map>
#include <string>
#include "matrix.hpp"

class Color
{
    // Each color channel is [0, 1) and can be rescaled to any color depth

public:
    enum Surface
    {
        SPECULAR = 0,
        DIFFUSE,
        DIELECTRIC,
    };

    static enum Surface stringToSurface(std::string str);

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

private:
    static const std::map<std::string, enum Surface> sSurfaceMap;
};