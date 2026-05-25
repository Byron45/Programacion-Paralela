#include <iostream>
#include <vector>

#include <omp.h>

#include <immintrin.h>

#include <fmt/core.h>
#include <fmt/ranges.h>

int operacionesVectoriales(const std::vector<float> &x, const std::vector<float> y)
{
    int num_elementos = x.size();
    int suma = 0;

    int tope = (num_elementos / 8) * 8;

    float sum_tmp[8];

    for (int i = 0; i < tope; i += 8)
    {
        __m256 mx = _mm256_loadu_ps(&x[i]);
        __m256 my = _mm256_loadu_ps(&y[i]);

        __m256 mz = _mm256_mul_ps(mx, my);

        _mm256_storeu_ps(sum_tmp, mz);

        for (int j = 0; j < 8; j++)
        {
            suma += sum_tmp[j];
        }
    }

    for (int i = tope; i < num_elementos; i++)
    {
        suma += x[i] * y[i];
    }

    return suma;
}

int escalar_secciones_paralelas(const std::vector<float> &x, const std::vector<float> y)
{
    int num_elementos = x.size();
    int suma = 0;

#pragma omp parallel shared(x, y, num_elementos, suma)
    {
        int thread_id = omp_get_thread_num();
        int thread_count = omp_get_num_threads();

        int block_size = std::ceil(1.0 * num_elementos / thread_count);

        int start = thread_id * block_size;
        int end = (thread_id + 1) * block_size;

        if (end > num_elementos)
        {
            end = num_elementos;
        }

        int local_sum = 0;
        for (int i = start; i < end; i++)
        {
            local_sum += x[i] * y[i];
        }

#pragma omp critical
        suma = suma + local_sum;
    }

    return suma;
}

int main()
{

    std::vector<float> x = {1, 2, 3, 4, 5};
    std::vector<float> y = {2, 4, 6, 8, 10};

    auto suma1 = operacionesVectoriales(x, y);
    fmt::println("Operaciones Vectoriales: {}", suma1);

    auto suma2 = escalar_secciones_paralelas(x, y);
    fmt::println("OpenMP: {}", suma2);
    return 0;
}
