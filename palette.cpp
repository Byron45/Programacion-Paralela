#include "palette.h"

uint32_t bswap32(uint32_t a)
{
    return ((a & 0x000000FF) << 24) |
           ((a & 0x0000FF00) << 8) |
           ((a & 0x00FF0000) >> 8) |
           ((a & 0xFF000000) >> 24);
}

std::vector<uint32_t> color_ramp = {
    bswap32(0xFF1010FF),

    bswap32(0xF31017FF),

    bswap32(0xE8101EFF),

    bswap32(0xDC1126FF),

    bswap32(0xD1112DFF),

    bswap32(0xC51235FF),

    bswap32(0xBA123CFF),

    bswap32(0xAE1343FF),

    bswap32(0xA3134BFF),

    bswap32(0x971452FF),

    bswap32(0x8C145AFF),

    bswap32(0x801461FF),

    bswap32(0x74ADD1FF),

    bswap32(0x5588BEFF),

    bswap32(0x3E60AAFF),

    bswap32(0x313695FF)};