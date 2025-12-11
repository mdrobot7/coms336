#pragma once
#include <map>
#include <string>
#include "vector.hpp"

// Typedef to make the param/return types clear
// Each color channel is [0, 1) and can be rescaled to any color depth

class Color : public Vector
{
public:
    using Vector::Vector;

    // Explicit conversion constructor, since it's a downcast
    Color(Vector &vec);

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
     * @return Color
     */
    static Color intToColor(int i);

    /**
     * @brief Convert a color vector to a 24-bit color.
     *
     * @param c
     * @return int
     */
    static int colorToInt(Color c);

    /**
     * @brief Bounce a colored ray off of a colored object.
     * Calculate the resulting color.
     *
     * @param surface
     * @param incoming
     * @return Color
     */
    static Color attenuate(Color surface, Color incoming);

    /**
     * @brief Send a colored ray through a dielectric (glass,
     * water, etc) and calculate how it refracts.
     *
     * @param surface
     * @param incoming
     * @return Color
     */
    static Color refract(Color surface, Color incoming);

private:
    static const std::map<std::string, enum Surface> sSurfaceMap;
};