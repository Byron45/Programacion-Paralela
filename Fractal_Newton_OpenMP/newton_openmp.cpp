#include "newton_openmp.h"
#include "palette.h"
#include <omp.h>
#include <complex>
#include <cmath>

#define NUM_RAICES 3
#define EPS 1e-4

extern int max_iteraciones;

static const std::complex<double> raices[NUM_RAICES] = {
    std::complex<double>(1.0, 0.0),
    std::complex<double>(-0.5, std::sqrt(3.0) / 2.0),
    std::complex<double>(-0.5, -std::sqrt(3.0) / 2.0)};

int newton_raphson_omp(std::complex<double> z, int &root)
{
    root = -1;

    int iter = 0;

    while (iter < max_iteraciones)
    {
        if (std::abs(z) > 2.0)
        {
            break;
        }

        for (int k = 0; k < NUM_RAICES; k++)
        {
            if (std::abs(z - raices[k]) < EPS)
            {
                root = k;
                return iter;
            }
        }

        std::complex<double> z2 = z * z;
        std::complex<double> denom = 3.0 * z2;

        if (std::abs(denom) < 1e-12)
        {
            break;
        }

        z = z - (z * z2 - 1.0) / denom;

        iter++;
    }

    return iter;
}

void newton_openmp_regiones(double x_min, double y_min, double x_max,
                            double y_max, uint32_t width, uint32_t height, uint32_t *pixel_buffer)
{
    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / height;

#pragma omp parallel
    {
        int thread_count = omp_get_num_threads();
        int thread_id = omp_get_thread_num();

        int delta = std::ceil(width * 1.0 / thread_count);

        int start = thread_id * delta;
        int end = (thread_id + 1) * delta;

        if (thread_id == thread_count - 1)
        {
            end = width;
        }

        for (int i = start; i < end; i++)
        {
            for (int j = 0; j < height; j++)
            {
                double x = x_min + i * dx;
                double y = y_max - j * dy;

                std::complex<double> z0(x, y);

                int root;
                int iter = newton_raphson_omp(z0, root);

                uint32_t color;
                if (root != -1)
                {
                    int index = (root * 5 + iter) % PALETTE_SIZE;
                    color = color_ramp[index];
                }
                else
                {
                    color = 0xFF000000;
                }

                pixel_buffer[j * width + i] = color;
            }
        }
    }
}

void newton_openmp_for(double x_min, double y_min, double x_max,
                       double y_max, uint32_t width, uint32_t height, uint32_t *pixel_buffer)
{
    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / height;

#pragma omp parallel for
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            double x = x_min + i * dx;
            double y = y_max - j * dy;

            std::complex<double> z0(x, y);

            int root;
            int iter = newton_raphson_omp(z0, root);

            uint32_t color;
            if (root != -1)
            {
                int index = (root * 5 + iter) % PALETTE_SIZE;
                color = color_ramp[index];
            }
            else
            {
                color = 0xFF000000;
            }

            pixel_buffer[j * width + i] = color;
        }
    }
}
