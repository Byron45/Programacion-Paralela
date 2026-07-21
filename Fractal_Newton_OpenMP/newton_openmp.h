#ifndef NEWTON_OPENMP_H
#define NEWTON_OPENMP_H

#include <cstdint>

void newton_openmp_regiones(double x_min, double y_min, double x_max,
                            double y_max, uint32_t width, uint32_t height, uint32_t *pixel_buffer);

void newton_openmp_for(double x_min, double y_min, double x_max,
                       double y_max, uint32_t width, uint32_t height, uint32_t *pixel_buffer);

#endif
