#ifndef NEWTON_SIMD_H
#define NEWTON_SIMD_H

#include <cstdint>

void newton_simd(double x_min, double y_min, double x_max,
                 double y_max, uint32_t width, uint32_t height, uint32_t *pixel_buffer);

#endif // NEWTON_SIMD_H
