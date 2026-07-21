#ifndef NEWTON_MPI_H
#define NEWTON_MPI_H

#include <cstdint>

#define NUM_RAICES 3

void newton_mpi(double x_min, double y_min, double x_max, double y_max,
                uint32_t width, uint32_t height,
                uint32_t row_start, uint32_t row_end,
                int max_iteraciones,
                uint32_t *pixel_buffer, double *total_iters_local);

#endif
