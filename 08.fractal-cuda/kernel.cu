const int PALETTE_SIZE = 16;

__constant__ // definida en la GPU
    unsigned int color_ramp[PALETTE_SIZE];

__device__
    uint32_t
    acotado(double x, double y, double cr, double ci, int max_iteraciones)
{

    int iter = 1;

    double zr = x;
    double zi = y;

    while (iter < max_iteraciones && (zr * zr + zi * zi) < 4.0)
    {
        double dr = zr * zr - zi * zi + cr;
        double di = 2.0 * zr * zi + ci;
        zr = dr;
        zi = di;

        iter++;
    }

    if (iter < max_iteraciones)
    {
        int index = iter % PALETTE_SIZE;
        return color_ramp[index];
    }

    return 0xFF000000; // negro
}

__global__ void julia_kernel(
    double centro_real, double centro_imag,
    int num_iterations,
    double x_min, double y_min, double x_max, double y_max,
    uint32_t width,
    uint32_t height,
    uint32_t *pixel_buffer)
{

    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / height;

    int index = blockIdx.x * blockDim.x + threadIdx.x;

    if (index < width * height)
    {
        int i = index % width;
        int j = index / width;

        // z = x+yi = (x,y)
        double x = x_min + i * dx;
        double y = y_max - j * dy;

        auto color = acotado(x, y,
                             centro_real,
                             centro_imag,
                             num_iterations);

        pixel_buffer[j * width + i] = color;
    }
}
//-----------------------------------------------------------

void copiar_paleta(unsigned int *h_palette)
{
    // copiar la paleta desde la CPU a la GPU
    cudaMemcpyToSymbol(color_ramp, h_palette, PALETTE_SIZE * sizeof(unsigned int));
}

void julia_gpu(
    double centro_real, double centro_imag,
    int num_iterations,
    double x_min, double y_min, double x_max, double y_max,
    uint32_t width,
    uint32_t height,
    uint32_t *pixel_buffer)
{

    int threads_per_block = 1024;
    int blocks_per_grid = std::ceil((width * height) * 1.0 / threads_per_block);

    julia_kernel<<<blocks_per_grid, threads_per_block>>>(
        centro_real, centro_imag, num_iterations,
        x_min, y_min, x_max, y_max,
        width, height,
        pixel_buffer);
}