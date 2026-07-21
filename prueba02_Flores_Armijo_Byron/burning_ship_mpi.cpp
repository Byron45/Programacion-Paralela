#include "burning_ship_mpi.h"
#include "palette.h"
#include <cmath>

int iteraciones_burning_ship(double cr, double ci, int max_iteraciones)
{
    int iter = 1;

    double zr = 0.0;
    double zi = 0.0;

    while (iter < max_iteraciones && (zr * zr + zi * zi) < 4.0)
    {
        double ar = std::abs(zr);
        double ai = std::abs(zi);

        double dr = ar * ar - ai * ai + cr;
        double di = 2.0 * ar * ai + ci;

        zr = dr;
        zi = di;

        iter++;
    }

    return iter;
}

void burning_ship_mpi(double x_min, double y_min, double x_max,
                      double y_max, uint32_t width, uint32_t height,
                      uint32_t row_start, uint32_t row_end,
                      int max_iteraciones,
                      uint32_t *pixel_buffer, int *local_hist)
{
    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / height;

    for (uint32_t j = row_start; j < row_end; j++)
    {
        for (uint32_t i = 0; i < width; i++)
        {
            double x = x_min + i * dx;
            double y = y_max - j * dy;

            int iter = iteraciones_burning_ship(x, y, max_iteraciones);

            uint32_t color;

            if (iter < max_iteraciones)
            {
                int index = iter % PALETTE_SIZE;
                color = color_ramp[index];

                int bin = iter * NBINS / max_iteraciones;
                if (bin >= NBINS)
                {
                    bin = NBINS - 1;
                }
                local_hist[bin]++;
            }
            else
            {
                color = 0xFF000000;
            }

            pixel_buffer[(j - row_start) * width + i] = color;
        }
    }
}
