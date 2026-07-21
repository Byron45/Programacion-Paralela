#include "newton_simd.h"
#include "palette.h"
#include <cstring>

#include <immintrin.h> //AVX

extern int max_iteraciones;

#define NUM_RAICES 3
#define EPS 1e-4f

static const float raiz_real[NUM_RAICES] = {1.0f, -0.5f, -0.5f};
static const float raiz_imag[NUM_RAICES] = {0.0f, 0.8660254f, -0.8660254f}; // sqrt(3)/2

void newton_simd(double x_min, double y_min, double x_max,
                 double y_max, uint32_t width, uint32_t height, uint32_t *pixel_buffer)
{
    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / height;

    __m256 xmin = _mm256_set1_ps((float)x_min);

    __m256 ymax = _mm256_set1_ps((float)y_max);

    __m256 xscale = _mm256_set1_ps((float)dx);
    __m256 yscale = _mm256_set1_ps((float)dy);

    __m256 r0_r = _mm256_set1_ps(raiz_real[0]);
    __m256 r0_i = _mm256_set1_ps(raiz_imag[0]);
    __m256 r1_r = _mm256_set1_ps(raiz_real[1]);
    __m256 r1_i = _mm256_set1_ps(raiz_imag[1]);
    __m256 r2_r = _mm256_set1_ps(raiz_real[2]);
    __m256 r2_i = _mm256_set1_ps(raiz_imag[2]);

    __m256 max_norma = _mm256_set1_ps(4.0f);
    __m256 eps2 = _mm256_set1_ps(EPS * EPS);
    __m256 cero = _mm256_set1_ps(0.0f);
    __m256 uno = _mm256_set1_ps(1.0f);
    __m256 dos = _mm256_set1_ps(2.0f);
    __m256 tres = _mm256_set1_ps(3.0f);
    __m256 medio = _mm256_set1_ps(0.5f);

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j += 8)
        {
            __m256 mx = _mm256_set1_ps((float)i);
            __m256 my = _mm256_set_ps(j + 7, j + 6, j + 5, j + 4, j + 3, j + 2, j + 1, j + 0);

            __m256 zr = _mm256_add_ps(xmin, _mm256_mul_ps(mx, xscale));
            __m256 zi = _mm256_sub_ps(ymax, _mm256_mul_ps(my, yscale));

            __m256 mk = cero;
            __m256 root_mas_1 = cero;
            __m256 acotado = _mm256_cmp_ps(uno, uno, _CMP_LE_OS);

            for (int iter = 0; iter < max_iteraciones; iter++)
            {
                __m256 zr2 = _mm256_mul_ps(zr, zr);
                __m256 zi2 = _mm256_mul_ps(zi, zi);
                __m256 norma2 = _mm256_add_ps(zr2, zi2);

                acotado = _mm256_and_ps(acotado, _mm256_cmp_ps(norma2, max_norma, _CMP_LE_OS));

                __m256 activo_antes = _mm256_and_ps(acotado, _mm256_cmp_ps(root_mas_1, medio, _CMP_LE_OS));

                __m256 dr0 = _mm256_sub_ps(zr, r0_r);
                __m256 di0 = _mm256_sub_ps(zi, r0_i);
                __m256 d0 = _mm256_add_ps(_mm256_mul_ps(dr0, dr0), _mm256_mul_ps(di0, di0));
                __m256 hit0 = _mm256_and_ps(_mm256_cmp_ps(d0, eps2, _CMP_LE_OS), activo_antes);
                root_mas_1 = _mm256_add_ps(root_mas_1, _mm256_and_ps(hit0, uno));

                __m256 dr1 = _mm256_sub_ps(zr, r1_r);
                __m256 di1 = _mm256_sub_ps(zi, r1_i);
                __m256 d1 = _mm256_add_ps(_mm256_mul_ps(dr1, dr1), _mm256_mul_ps(di1, di1));
                __m256 hit1 = _mm256_and_ps(_mm256_cmp_ps(d1, eps2, _CMP_LE_OS), activo_antes);
                root_mas_1 = _mm256_add_ps(root_mas_1, _mm256_and_ps(hit1, dos));

                __m256 dr2 = _mm256_sub_ps(zr, r2_r);
                __m256 di2 = _mm256_sub_ps(zi, r2_i);
                __m256 d2 = _mm256_add_ps(_mm256_mul_ps(dr2, dr2), _mm256_mul_ps(di2, di2));
                __m256 hit2 = _mm256_and_ps(_mm256_cmp_ps(d2, eps2, _CMP_LE_OS), activo_antes);
                root_mas_1 = _mm256_add_ps(root_mas_1, _mm256_and_ps(hit2, tres));

                __m256 activo_despues = _mm256_and_ps(acotado, _mm256_cmp_ps(root_mas_1, medio, _CMP_LE_OS));

                mk = _mm256_add_ps(mk, _mm256_and_ps(activo_despues, uno));

                __m256 z2r = _mm256_sub_ps(zr2, zi2);
                __m256 zrzi = _mm256_mul_ps(zr, zi);
                __m256 z2i = _mm256_add_ps(zrzi, zrzi);

                __m256 z3r = _mm256_sub_ps(_mm256_mul_ps(z2r, zr), _mm256_mul_ps(z2i, zi));
                __m256 z3i = _mm256_add_ps(_mm256_mul_ps(z2r, zi), _mm256_mul_ps(z2i, zr));

                __m256 den_r = _mm256_mul_ps(tres, z2r);
                __m256 den_i = _mm256_mul_ps(tres, z2i);
                __m256 den2 = _mm256_add_ps(_mm256_mul_ps(den_r, den_r), _mm256_mul_ps(den_i, den_i));

                __m256 num_r = _mm256_sub_ps(z3r, uno);
                __m256 num_i = z3i;

                __m256 qr = _mm256_div_ps(_mm256_add_ps(_mm256_mul_ps(num_r, den_r), _mm256_mul_ps(num_i, den_i)), den2);
                __m256 qi = _mm256_div_ps(_mm256_sub_ps(_mm256_mul_ps(num_i, den_r), _mm256_mul_ps(num_r, den_i)), den2);

                zr = _mm256_sub_ps(zr, qr);
                zi = _mm256_sub_ps(zi, qi);
            }

            float mk_arr[8];
            float root_arr[8];
            _mm256_storeu_ps(mk_arr, mk);
            _mm256_storeu_ps(root_arr, root_mas_1);

            for (int it = 0; it < 8; it++)
            {
                int index = (j + it) * width + i;
                if (index < width * height)
                {
                    if (root_arr[it] > 0.5f)
                    {
                        int root = (int)root_arr[it] - 1;
                        int iter = (int)mk_arr[it];
                        int color_index = (root * 5 + iter) % PALETTE_SIZE;
                        pixel_buffer[index] = color_ramp[color_index];
                    }
                    else
                    {
                        pixel_buffer[index] = 0xFF000000;
                    }
                }
            }
        }
    }
}
