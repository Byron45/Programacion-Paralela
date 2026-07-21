#include <cstdint>
#include <cmath>

#define PALETTE_SIZE 16
#define NUM_RAICES 3
#define EPS 1e-4

__constant__ unsigned int color_ramp[PALETTE_SIZE];
__constant__ double raiz_real[NUM_RAICES];
__constant__ double raiz_imag[NUM_RAICES];

__device__ int newton_raphson_gpu(double x, double y, int max_iteraciones, int &root)
{
    root = -1;

    double zr = x;
    double zi = y;

    int iter = 0;

    while (iter < max_iteraciones)
    {
        if ((zr * zr + zi * zi) > 4.0)
        {
            break;
        }

        for (int k = 0; k < NUM_RAICES; k++)
        {
            double dr = zr - raiz_real[k];
            double di = zi - raiz_imag[k];
            if ((dr * dr + di * di) < (EPS * EPS))
            {
                root = k;
                return iter;
            }
        }

        double zr2 = zr * zr - zi * zi;
        double zi2 = 2.0 * zr * zi;

        double zr3 = zr2 * zr - zi2 * zi;
        double zi3 = zr2 * zi + zi2 * zr;

        double dr = 3.0 * zr2;
        double di = 3.0 * zi2;
        double denom2 = dr * dr + di * di;

        if (denom2 < 1e-24)
        {
            break;
        }

        double nr = zr3 - 1.0;
        double ni = zi3;

        double qr = (nr * dr + ni * di) / denom2;
        double qi = (ni * dr - nr * di) / denom2;

        zr -= qr;
        zi -= qi;

        iter++;
    }

    return iter;
}

__global__ void newton_kernel(
    double x_min, double y_min, double x_max, double y_max,
    uint32_t width, uint32_t height,
    int max_iteraciones,
    uint32_t *pixel_buffer,
    unsigned long long *total_iters)
{
    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / height;

    int index = blockIdx.x * blockDim.x + threadIdx.x;

    if (index < width * height)
    {
        int i = index % width;
        int j = index / width;

        double x = x_min + i * dx;
        double y = y_max - j * dy;

        int root;
        int iter = newton_raphson_gpu(x, y, max_iteraciones, root);

        uint32_t color;
        if (root != -1)
        {
            int idx = (root * 5 + iter) % PALETTE_SIZE;
            color = color_ramp[idx];
        }
        else
        {
            color = 0xFF000000;
        }

        pixel_buffer[j * width + i] = color;

        atomicAdd(total_iters, (unsigned long long)iter);
    }
}

void copiar_paleta(unsigned int *h_palette)
{
    cudaMemcpyToSymbol(color_ramp, h_palette, PALETTE_SIZE * sizeof(unsigned int));
}

void copiar_raices(double *h_real, double *h_imag)
{
    cudaMemcpyToSymbol(raiz_real, h_real, NUM_RAICES * sizeof(double));
    cudaMemcpyToSymbol(raiz_imag, h_imag, NUM_RAICES * sizeof(double));
}

void newton_gpu(
    double x_min, double y_min, double x_max, double y_max,
    uint32_t width, uint32_t height,
    int max_iteraciones,
    uint32_t *pixel_buffer,
    unsigned long long *d_total_iters)
{
    int threads_per_block = 1024;
    int blocks_per_grid = (int)std::ceil((width * height) * 1.0 / threads_per_block);

    cudaMemset(d_total_iters, 0, sizeof(unsigned long long));

    newton_kernel<<<blocks_per_grid, threads_per_block>>>(
        x_min, y_min, x_max, y_max,
        width, height,
        max_iteraciones,
        pixel_buffer,
        d_total_iters);
}
