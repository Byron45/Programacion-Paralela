#ifndef BLUR_OPENMP_H
#define BLUR_OPENMP_H

#include <cstdint>
#include <vector>

void generar_pesos_gaussianos(int k, std::vector<float> &pesos);

void blur_openmp_regiones(const uint8_t *src, uint8_t *dst,
                          int width, int height, int k,
                          const std::vector<float> &pesos);

#endif
