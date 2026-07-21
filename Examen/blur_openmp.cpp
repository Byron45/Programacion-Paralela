#include "blur_openmp.h"
#include <omp.h>
#include <cmath>

void generar_pesos_gaussianos(int k, std::vector<float> &pesos)
{
    int M = 2 * k + 1;
    std::vector<float> fila1d(M);

    // Fila 2k del triangulo de Pascal: C(2k, i)
    for (int i = 0; i <= 2 * k; i++)
    {
        float c = 1.0f;
        for (int t = 0; t < i; t++)
        {
            c = c * (2 * k - t) / (t + 1);
        }
        fila1d[i] = c;
    }

    // kernel 2D = producto externo de la fila 1D consigo misma
    pesos.resize(M * M);
    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < M; j++)
        {
            pesos[i * M + j] = fila1d[i] * fila1d[j];
        }
    }
}

// Calcula el valor filtrado de UN canal (R, G, B o A) para el pixel (x,y),
// ignorando el padding: solo suma vecinos validos y normaliza con la suma
// de esos pesos (no siempre la suma total del kernel).
static uint8_t procesar_pixel_canal(const uint8_t *src, int width, int height,
                                    int x, int y, int canal, int k,
                                    const std::vector<float> &pesos)
{
    int M = 2 * k + 1;

    float suma = 0.0f;
    float peso_total = 0.0f;

    for (int i = -k; i <= k; i++)
    {
        int ny = y + i;
        if (ny < 0 || ny >= height)
            continue; // fuera de la imagen: se ignora (sin padding)

        for (int j = -k; j <= k; j++)
        {
            int nx = x + j;
            if (nx < 0 || nx >= width)
                continue;

            float w = pesos[(i + k) * M + (j + k)];
            int nidx = (ny * width + nx) * 4 + canal;

            suma += w * src[nidx];
            peso_total += w;
        }
    }

    return (uint8_t)(suma / peso_total + 0.5f); // +0.5f para redondear
}

void blur_openmp_regiones(const uint8_t *src, uint8_t *dst,
                          int width, int height, int k,
                          const std::vector<float> &pesos)
{
#pragma omp parallel
    {
        int thread_count = omp_get_num_threads();
        int thread_id = omp_get_thread_num();

        // division manual del rango de FILAS por hilo (igual que
        // julia_openmp_regiones en 01.fractal-julia) -> NO se usa
        // #pragma omp parallel for ni #pragma omp for
        int delta = std::ceil(height * 1.0 / thread_count);
        int start = thread_id * delta;
        int end = (thread_id + 1) * delta;

        // IMPORTANTE: se recorta 'end' contra 'height' para TODOS los hilos,
        // no solo el ultimo. Si solo se recortara el ultimo (como en
        // julia_openmp_regiones de 01.fractal-julia), cuando 'height' no es
        // multiplo exacto de thread_count el PENULTIMO hilo puede quedarse
        // con end > height y escribir fuera del buffer (memoria corrupta).
        if (end > height)
        {
            end = height;
        }

        for (int y = start; y < end; y++)
        {
            for (int x = 0; x < width; x++)
            {
                for (int canal = 0; canal < 4; canal++) // R,G,B,A independientes
                {
                    int idx = (y * width + x) * 4 + canal;
                    dst[idx] = procesar_pixel_canal(src, width, height, x, y, canal, k, pesos);
                }
            }
        }

    } // cierre del parallel
}
