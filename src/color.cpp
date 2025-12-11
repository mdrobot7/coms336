#include <stdexcept>
#include "color.hpp"

const std::map<std::string, enum Color::Surface> Color::sSurfaceMap = {
    {"specular", Color::SPECULAR},
    {"diffuse", Color::DIFFUSE},
    {"dielectric", Color::DIELECTRIC},
    {"emissive", Color::EMISSIVE},
};

enum Color::Surface Color::stringToSurface(std::string str)
{
    auto val = sSurfaceMap.find(str);
    if (val != sSurfaceMap.end())
    {
        return val->second;
    }
    else
    {
        throw std::invalid_argument("Invalid surface type");
    }
}

Color Color::intToColor(int i)
{
    return Color(((i & 0xFF0000) >> 16) / 256.0, ((i & 0xFF00) >> 8) / 256.0, (i & 0xFF) / 256.0);
}

int Color::colorToInt(Color c)
{
    return (((uint8_t)(c[0] * 256)) << 16) | (((uint8_t)(c[1] * 256)) << 8) | ((uint8_t)(c[2] * 256));
}

Color Color::attenuate(Color surface, Color incoming)
{
    return Color(surface[0] * incoming[0], surface[1] * incoming[1], surface[2] * incoming[2]);
}

Color Color::refract(Color surface, Color incoming)
{
    // TODO this is hard
    return incoming;
}