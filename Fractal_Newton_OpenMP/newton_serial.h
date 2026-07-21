#ifndef NEWTON_SERIAL_H
#define NEWTON_SERIAL_H

#include <cstdint>

#define NUM_RAICES 3

void newton_serial(double x_min, double y_min, double x_max,
                   double y_max, uint32_t width, uint32_t height, uint32_t *pixel_buffer);

#endif
