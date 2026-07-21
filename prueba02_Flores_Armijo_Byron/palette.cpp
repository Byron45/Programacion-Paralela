#include "palette.h"

uint32_t bswap32(uint32_t a)
{
    return ((a & 0x000000FF) << 24) |
           ((a & 0x0000FF00) << 8) |
           ((a & 0x00FF0000) >> 8) |
           ((a & 0xFF000000) >> 24);
}

std::vector<uint32_t> color_ramp = {
    bswap32(0x00081AFF),
    bswap32(0x001030FF),
    bswap32(0x001C4DFF),
    bswap32(0x002E66FF),
    bswap32(0x004080FF),
    bswap32(0x00589FFF),
    bswap32(0x0070BFFF),
    bswap32(0x0090D9FF),
    bswap32(0x00B0F0FF),
    bswap32(0x30CCFFFF),
    bswap32(0x60DFFFFF),
    bswap32(0x90EAFFFF),
    bswap32(0xB8F2FFFF),
    bswap32(0xD8F8FFFF),
    bswap32(0xF0FCFFFF),
    bswap32(0xFFFFFFFF)};