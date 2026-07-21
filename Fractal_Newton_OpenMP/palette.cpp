#include "palette.h"

uint32_t bswap32(uint32_t a)
{
    return ((a & 0x000000FF) << 24) |
           ((a & 0x0000FF00) << 8) |
           ((a & 0x00FF0000) >> 8) |
           ((a & 0xFF000000) >> 24);
}

std::vector<uint32_t> color_ramp = {
    bswap32(0xFF0000FF),
    bswap32(0xFF4000FF),
    bswap32(0xFF8000FF),
    bswap32(0xFFBF00FF),
    bswap32(0xFFFF00FF),
    bswap32(0xBFFF00FF),
    bswap32(0x80FF00FF),
    bswap32(0x40FF00FF),
    bswap32(0x00FF40FF),
    bswap32(0x00FF80FF),
    bswap32(0x00FFBFFF),
    bswap32(0x00FFFFFF),
    bswap32(0x00BFFFFF),
    bswap32(0x0080FFFF),
    bswap32(0x0040FFFF),
    bswap32(0x4000FFFF)};
