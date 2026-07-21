#include <iostream>
#include <cstdint>
#include <cstring>
#include <fmt/core.h>
#include <SFML/Graphics.hpp>

#include "palette.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <cuda_runtime.h>

double x_min = -1.5;
double x_max = 1.5;
double y_min = -1.0;
double y_max = 1.0;

int max_iteraciones = 50;

double raiz_real[3] = {1.0, -0.5, -0.5};
double raiz_imag[3] = {0.0, 0.8660254037844386, -0.8660254037844386};

#define WIDTH 1600
#define HEIGHT 900

uint32_t *host_pixel_buffer = nullptr;
uint32_t *device_pixel_buffer = nullptr;
unsigned long long *device_total_iters = nullptr;

#define CHECK(expr)                                                                                          \
    {                                                                                                        \
        auto internal_error = (expr);                                                                        \
        if (internal_error != cudaSuccess)                                                                   \
        {                                                                                                    \
            fmt::println("{}: {} in {} at line {}", (int)internal_error, cudaGetErrorString(internal_error), \
                         __FILE__, __LINE__);                                                                \
            exit(EXIT_FAILURE);                                                                              \
        }                                                                                                    \
    }

extern void copiar_paleta(unsigned int *h_palette);
extern void copiar_raices(double *h_real, double *h_imag);

extern void newton_gpu(
    double x_min, double y_min, double x_max, double y_max,
    uint32_t width, uint32_t height,
    int max_iteraciones,
    uint32_t *pixel_buffer,
    unsigned long long *d_total_iters);

int main()
{
    int deviceId = 0;
    cudaSetDevice(deviceId);

    cudaDeviceProp deviceProp;
    cudaGetDeviceProperties(&deviceProp, deviceId);

    fmt::println("Device {}:", deviceProp.name);
    fmt::println("Total memory: {} MB", deviceProp.totalGlobalMem / 1024.0 / 1024.0);
    fmt::println("Multiprocessor count: {}", deviceProp.multiProcessorCount);

    size_t buffer_size = WIDTH * HEIGHT * sizeof(uint32_t);
    host_pixel_buffer = (uint32_t *)malloc(buffer_size);
    std::memset(host_pixel_buffer, 0, buffer_size);

    CHECK(cudaMalloc(&device_pixel_buffer, buffer_size));
    CHECK(cudaMalloc(&device_total_iters, sizeof(unsigned long long)));

    copiar_paleta(color_ramp.data());
    copiar_raices(raiz_real, raiz_imag);

    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Newton Fractal - CUDA");

#ifdef _WIN32
    HWND hwnd = window.getNativeHandle();
    ShowWindow(hwnd, SW_MAXIMIZE);
#endif

    sf::Texture texture({WIDTH, HEIGHT});
    sf::Sprite sprite(texture);

    sf::Font font("arial.ttf");
    sf::Text text(font, "Newton Fractal - CUDA", 20);
    text.setFillColor(sf::Color::White);
    text.setPosition({10, 10});
    text.setStyle(sf::Text::Bold);

    std::string options = "Controles: [Up/Down] Cambiar iteraciones | [Esc] Salir";
    sf::Text textOptions(font, options, 18);
    textOptions.setFillColor(sf::Color::White);
    textOptions.setStyle(sf::Text::Bold);
    textOptions.setPosition({10, window.getView().getSize().y - 40});

    // FPS
    int frames = 0;
    int fps = 0;
    sf::Clock clock;

    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
            else if (event->is<sf::Event::KeyReleased>())
            {
                auto evt = event->getIf<sf::Event::KeyReleased>();

                switch (evt->scancode)
                {
                case sf::Keyboard::Scan::Up:
                    max_iteraciones += 10;
                    break;
                case sf::Keyboard::Scan::Down:
                    max_iteraciones -= 10;
                    if (max_iteraciones < 10)
                        max_iteraciones = 10;
                    break;
                case sf::Keyboard::Scan::Escape:
                    window.close();
                    break;
                default:
                    break;
                }
            }
        }

        cudaEventRecord(start);

        newton_gpu(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, max_iteraciones,
                   device_pixel_buffer, device_total_iters);

        cudaEventRecord(stop);
        CHECK(cudaGetLastError());

        CHECK(cudaMemcpy(host_pixel_buffer, device_pixel_buffer, buffer_size, cudaMemcpyDeviceToHost));

        unsigned long long total_iters = 0;
        CHECK(cudaMemcpy(&total_iters, device_total_iters, sizeof(unsigned long long), cudaMemcpyDeviceToHost));

        cudaEventSynchronize(stop);
        float compute_ms = 0.0f;
        cudaEventElapsedTime(&compute_ms, start, stop);

        texture.update((const uint8_t *)host_pixel_buffer);

        // contar FPS
        frames++;
        if (clock.getElapsedTime().asSeconds() >= 1.0f)
        {
            fps = frames;
            frames = 0;
            clock.restart();
        }

        auto msg = fmt::format(
            "Newton Fractal - CUDA\n"
            "Iteraciones: {}\n"
            "compute_ms (GPU): {:.3f}\n"
            "total_iters: {}\n"
            "FPS: {}",
            max_iteraciones, compute_ms, total_iters, fps);
        text.setString(msg);

        window.clear();
        window.draw(sprite);
        window.draw(text);
        window.draw(textOptions);
        window.display();
    }

    cudaEventDestroy(start);
    cudaEventDestroy(stop);

    cudaFree(device_pixel_buffer);
    cudaFree(device_total_iters);
    free(host_pixel_buffer);

    return 0;
}
