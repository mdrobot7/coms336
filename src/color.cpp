#include "color.hpp"

vector_t Color::intToColor(int i)
{
    return vector_t{((i & 0xFF0000) >> 16) / 256.0, ((i & 0xFF00) >> 8) / 256.0, (i & 0xFF) / 256.0};
}

int Color::colorToInt(vector_t c)
{
    return (((uint8_t)(c[0] * 256)) << 16) | (((uint8_t)(c[1] * 256)) << 8) | ((uint8_t)(c[2] * 256));
}

vector_t Color::attenuate(vector_t surface, vector_t incoming)
{
    return vector_t{surface[0] * incoming[0], surface[1] * incoming[1], surface[2] * incoming[2]};
}

vector_t Color::refract(vector_t surface, vector_t incoming)
{
    // TODO this is hard
    return incoming;
}