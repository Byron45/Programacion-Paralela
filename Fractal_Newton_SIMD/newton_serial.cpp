#include "newton_serial.h"
#include "palette.h"
#include <complex>
#include <cmath>

#define EPS 1e-4

extern int max_iteraciones;

static const std::complex<double> raices[NUM_RAICES] = {
    std::complex<double>(1.0, 0.0),
    std::complex<double>(-0.5, std::sqrt(3.0) / 2.0),
    std::complex<double>(-0.5, -std::sqrt(3.0) / 2.0)};

int newton_raphson(std::complex<double> z, int &root)
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

void newton_serial(double x_min, double y_min, double x_max,
                   double y_max, uint32_t width, uint32_t height, uint32_t *pixel_buffer)
{
    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / height;

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            double x = x_min + i * dx;
            double y = y_max - j * dy;

            std::complex<double> z0(x, y);

            int root;
            int iter = newton_raphson(z0, root);

            uint32_t color;
            if (root != -1)
            {
                int index = (root * 5 + iter) % PALETTE_SIZE;
                color = color_ramp[index];
            }
            else
            {
                color = 0xFF000000; // negro
            }

            pixel_buffer[j * width + i] = color;
        }
    }
}
