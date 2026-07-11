#include <iostream>
#include <fmt/core.h>
#include <SFML/Graphics.hpp>
#include <complex>
#include "palette.h"
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#endif

#include <cuda_runtime.h>

// Parametros
double x_min = -1.5;
double x_max = 1.5;
double y_min = -1.0;
double y_max = 1.0;

int max_iteraciones = 10;

std::complex<double> c(-0.7, 0.27015);

// Dimension de la imagen
#define WIDTH 1920
#define HEIGHT 1080

// Textura
uint32_t *host_pixel_buffer = nullptr;   // entero sin signo de 32 bits
uint32_t *device_pixel_buffer = nullptr; // WxH

#define CHECK(expr) {                               \
        auto internal_error = (expr);               \
        if (internal_error!=cudaSuccess) {          \
            fmt::println("{}: {} in {} at line {}", (int )internal_error, cudaGetErrorString(internal_error), __FILE__, __LINE__);    \
            exit(EXIT_FAILURE);                     \
        }                                           \
    }

extern void copiar_paleta(unsigned int *h_palette);

extern void julia_gpu(
    double centro_real, double centro_imag,
    int num_iterations,
    double x_min, double y_min, double x_max, double y_max,
    uint32_t width,
    uint32_t height,
    uint32_t *pixel_buffer);

int main()
{

    int deviceId = 0;

    cudaSetDevice(deviceId);

    cudaDeviceProp deviceProp;
    cudaGetDeviceProperties(&deviceProp, deviceId);

    fmt::println("Device {}:", deviceProp.name);
    fmt::println("Total memory: {} MB", deviceProp.totalGlobalMem / 1024.0 / 1024.0);

    //--inicializar
    size_t buffer_size = WIDTH * HEIGHT * sizeof(uint32_t); // 1600*900*4
    host_pixel_buffer = (uint32_t *)malloc(buffer_size);
    std::memset(host_pixel_buffer, 0, buffer_size);

    CHECK(cudaMalloc(&device_pixel_buffer, buffer_size)); // Macro para verificar errores
    copiar_paleta(color_ramp.data());

    //--inicializar UI
    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Julia Set - SFML");

#ifdef _WIN32
    HWND hwnd = window.getNativeHandle();
    ShowWindow(hwnd, SW_MAXIMIZE);
#endif

    sf::Vector2u windowSize = {WIDTH, HEIGHT};

    sf::Texture texture(windowSize);
    sf::Sprite sprite(texture);

    sf::Font font("arial.ttf");
    sf::Text text(font, "Fractal CUDA", 24);
    text.setFillColor(sf::Color::White);
    text.setPosition({10, 10});
    text.setStyle(sf::Text::Bold);

    std::string options = "Up/Down: Change Iterations";
    sf::Text textOptions(font, options, 20);
    textOptions.setFillColor(sf::Color::White);
    textOptions.setStyle(sf::Text::Bold);
    textOptions.setPosition({10, window.getView().getSize().y - 40});

    // FPS
    int frames = 0;
    int fps = 0;
    sf::Clock clock;

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
                }
            }

        std:
            memset(host_pixel_buffer, 0, WIDTH * HEIGHT * sizeof(uint32_t));
        }

        std::string mode = "";

        mode = "GPU CUDA";
        // dibujar en la GPU
        // invocar kernel
        // copiar la imagen de la GPU a la CPU
        julia_gpu(c.real(), c.imag(), max_iteraciones, x_min, y_min, x_max, y_max, WIDTH, HEIGHT, device_pixel_buffer);
        CHECK(cudaGetLastError());
        CHECK(cudaMemcpy(host_pixel_buffer, device_pixel_buffer, buffer_size, cudaMemcpyDeviceToHost));

        // dibujar fractal
        // pintamos

        // crear la textura
        texture.update((const uint8_t *)host_pixel_buffer);

        // contar FPS
        frames++;

        if (clock.getElapsedTime().asSeconds() >= 1.0f)
        {
            fps = frames;
            frames = 0;
            clock.restart();
        }

        // actualizar el titulo de la ventana con el FPS
        auto msg = fmt::format("Julia GPU: iterations {}. FPS: {}. Mode: {}", max_iteraciones, fps, mode);
        text.setString(msg);

        window.clear();

        {
            window.draw(sprite);
            window.draw(text);
            window.draw(textOptions);
        }

        window.display();
    }

    return 0;
}