#include <omp.h>
#include <math.h>
#include <stdint.h>

// Constantes
#define PALETTE_SIZE 16

// Función para calcular el número de iteraciones necesarias hasta escapar
static inline int boundingIterations(double x, double y, int maxIter,
                                     double cReal, double cImag) {
    int iter = 1;
    double zr = x;
    double zi = y;

    while (iter < maxIter && (zr * zr + zi * zi) < 4.0) {
        double dr = zr * zr - zi * zi + cReal;
        double di = 2.0 * zr * zi + cImag;

        zr = dr;
        zi = di;
        iter++;
    }

    return iter;
}

/**
 * Calcula el conjunto de Julia usando OpenMP para paralelización
 * Similar a la implementación Java con FractalThreads
 *
 * @param xMin, yMin, xMax, yMax: Límites del área a renderizar
 * @param width, height: Resolución de la imagen
 * @param maxIterations: Máximo número de iteraciones
 * @param cReal, cImag: Parámetro complejo c del conjunto de Julia
 * @param colorRamp: Paleta de colores (PALETTE_SIZE elementos)
 * @param pixelBuffer: Buffer de salida (width * height * 4 bytes RGBA)
 */
void julia_openmp(double xMin, double yMin, double xMax, double yMax,
                  int width, int height, int maxIterations,
                  double cReal, double cImag,
                  const uint32_t* colorRamp,
                  uint32_t* pixelBuffer) {

    double dx = (xMax - xMin) / width;
    double dy = (yMax - yMin) / height;

    // Paralelización estática (equivalente a Java Threads)
    #pragma omp parallel for collapse(2) schedule(static)
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            double x = xMin + i * dx;
            double y = yMax - j * dy;

            int iterations = boundingIterations(x, y, maxIterations, cReal, cImag);

            uint32_t color;
            if (iterations < maxIterations) {
                int colorIndex = iterations % PALETTE_SIZE;
                color = colorRamp[colorIndex];
            } else {
                color = 0xFF000000; // Negro para puntos dentro del conjunto
            }

            pixelBuffer[j * width + i] = color;
        }
    }
}

/**
 * Versión con scheduling dinámico para cargas no uniformes
 * (En caso de que las iteraciones varíen significativamente por región)
 */
void julia_openmp_dynamic(double xMin, double yMin, double xMax, double yMax,
                          int width, int height, int maxIterations,
                          double cReal, double cImag,
                          const uint32_t* colorRamp,
                          uint32_t* pixelBuffer) {

    double dx = (xMax - xMin) / width;
    double dy = (yMax - yMin) / height;

    // Paralelización dinámica para mejor balanceo de carga
    #pragma omp parallel for collapse(2) schedule(dynamic, 64)
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            double x = xMin + i * dx;
            double y = yMax - j * dy;

            int iterations = boundingIterations(x, y, maxIterations, cReal, cImag);

            uint32_t color;
            if (iterations < maxIterations) {
                int colorIndex = iterations % PALETTE_SIZE;
                color = colorRamp[colorIndex];
            } else {
                color = 0xFF000000;
            }

            pixelBuffer[j * width + i] = color;
        }
    }
}

/**
 * Versión con paralelización por filas (aprovecha better cache locality)
 */
void julia_openmp_rows(double xMin, double yMin, double xMax, double yMax,
                       int width, int height, int maxIterations,
                       double cReal, double cImag,
                       const uint32_t* colorRamp,
                       uint32_t* pixelBuffer) {

    double dx = (xMax - xMin) / width;
    double dy = (yMax - yMin) / height;

    // Paralelizar solo el loop externo (filas)
    #pragma omp parallel for schedule(guided)
    for (int i = 0; i < width; ++i) {
        double x = xMin + i * dx;

        for (int j = 0; j < height; ++j) {
            double y = yMax - j * dy;

            int iterations = boundingIterations(x, y, maxIterations, cReal, cImag);

            uint32_t color;
            if (iterations < maxIterations) {
                int colorIndex = iterations % PALETTE_SIZE;
                color = colorRamp[colorIndex];
            } else {
                color = 0xFF000000;
            }

            pixelBuffer[j * width + i] = color;
        }
    }
}

/**
 * Macro para compilación con OpenMP:
 *
 * GCC/Clang:
 *   -fopenmp
 *
 * MSVC:
 *   /openmp
 *
 * Ejemplo de compilación (librería DLL para Windows):
 *   cl /openmp /LD /EHsc fractal_openmp.cpp /link kernel32.lib
 */

