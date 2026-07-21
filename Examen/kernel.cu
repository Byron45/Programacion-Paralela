#include <cstdint>
#include <cmath>

/
#define MAX_KERNEL_SIZE 81

    __constant__ float pesos_gaussianos[MAX_KERNEL_SIZE];

__global__ void blur_kernel(
    const uint8_t *src,
    uint8_t *dst,
    int width,
    int height,
    int k)
{
    int M = 2 * k + 1;

    int index = blockIdx.x * blockDim.x + threadIdx.x;

    if (index < width * height)
    {
        int x = index % width;
        int y = index / width;

        for (int canal = 0; canal < 4; canal++)
        {
            float suma = 0.0f;
            float peso_total = 0.0f;

            for (int i = -k; i <= k; i++)
            {
                int ny = y + i;
                if (ny < 0 || ny >= height)
                    continue;

                for (int j = -k; j <= k; j++)
                {
                    int nx = x + j;
                    if (nx < 0 || nx >= width)
                        continue;

                    float w = pesos_gaussianos[(i + k) * M + (j + k)];
                    int nidx = (ny * width + nx) * 4 + canal;

                    suma += w * src[nidx];
                    peso_total += w;
                }
            }

            int idx = (y * width + x) * 4 + canal;
            dst[idx] = (uint8_t)(suma / peso_total + 0.5f);
        }
    }
}

void copiar_pesos(float *h_pesos, int n)
{
    cudaMemcpyToSymbol(pesos_gaussianos, h_pesos, n * sizeof(float));
}

void blur_gpu(
    const uint8_t *d_src,
    uint8_t *d_dst,
    int width,
    int height,
    int k)
{
    int threads_per_block = 1024;
    int blocks_per_grid = std::ceil((width * height) * 1.0 / threads_per_block);

    blur_kernel<<<blocks_per_grid, threads_per_block>>>(d_src, d_dst, width, height, k);
}
