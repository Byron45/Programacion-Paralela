#ifndef BURNING_SHIP_MPI_H
#define BURNING_SHIP_MPI_H

#include <cstdint>

#define NBINS 16

void burning_ship_mpi(double x_min, double y_min, double x_max,
                      double y_max, uint32_t width, uint32_t height,
                      uint32_t row_start, uint32_t row_end,
                      int max_iteraciones,
                      uint32_t *pixel_buffer, int *local_hist);

#endif
