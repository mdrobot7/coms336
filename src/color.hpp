#pragma once
#include <cstdint>
#include <vector>
#include <map>
#include <string>
#include "matrix.hpp"

// Typedef to make the param/return types clear
// Each color channel is [0, 1) and can be rescaled to any color depth
typedef vector_t color_t;

class Color
{
public:
    enum Surface
    {
        SPECULAR = 0,
        DIFFUSE,
        DIELECTRIC,
        EMISSIVE,
    };

    static enum Surface stringToSurface(std::string str);

    /**
     * @brief Convert a 24-bit color to a color vector.
     *
     * @param i
     * @return color_t
     */
    static color_t intToColor(int i);

    /**
     * @brief Convert a color vector to a 24-bit color.
     *
     * @param c
     * @return int
     */
    static int colorToInt(color_t c);

    /**
     * @brief Bounce a colored ray off of a colored object.
     * Calculate the resulting color.
     *
     * @param surface
     * @param incoming
     * @return color_t
     */
    static color_t attenuate(color_t surface, color_t incoming);

    /**
     * @brief Send a colored ray through a dielectric (glass,
     * water, etc) and calculate how it refracts.
     *
     * @param surface
     * @param incoming
     * @return color_t
     */
    static color_t refract(color_t surface, color_t incoming);

private:
    static const std::map<std::string, enum Surface> sSurfaceMap;
};