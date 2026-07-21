#include <iostream>
#include <fmt/core.h>
#include <SFML/Graphics.hpp>
#include <cstring>
#include <vector>

#include <omp.h>
#include <cuda_runtime.h>

#include "blur_openmp.h"

#ifdef _WIN32
#include <windows.h>
#endif

static const int M = 3;
static const int K = (M - 1) / 2;

static const char *IMAGE_FILENAME = "aucas.png";

enum class runtime_type
{
    OPENMP = 0,
    CUDA
};

#define CHECK(expr)                                                               \
    {                                                                             \
        auto internal_error = (expr);                                             \
        if (internal_error != cudaSuccess)                                        \
        {                                                                         \
            fmt::println("{}: {} in {} at line {}", (int)internal_error,          \
                         cudaGetErrorString(internal_error), __FILE__, __LINE__); \
            exit(EXIT_FAILURE);                                                   \
        }                                                                         \
    }

extern void copiar_pesos(float *h_pesos, int n);
extern void blur_gpu(const uint8_t *d_src, uint8_t *d_dst, int width, int height, int k);

int main()
{
    sf::Image im;
    if (!im.loadFromFile(IMAGE_FILENAME))
    {
        fmt::println("No se pudo cargar la imagen '{}'", IMAGE_FILENAME);
        return -1;
    }

    const std::uint8_t *ptr = im.getPixelsPtr();

    auto tam = im.getSize();
    unsigned int WIDTH = tam.x;
    unsigned int HEIGHT = tam.y;

    size_t buffer_size = (size_t)WIDTH * HEIGHT * 4;

    uint8_t *original_buffer = new uint8_t[buffer_size];
    uint8_t *display_buffer = new uint8_t[buffer_size];

    std::memcpy(original_buffer, ptr, buffer_size);
    std::memcpy(display_buffer, ptr, buffer_size);

    std::vector<float> pesos;
    generar_pesos_gaussianos(K, pesos);

    int deviceId = 0;
    cudaSetDevice(deviceId);

    cudaDeviceProp deviceProp;
    cudaGetDeviceProperties(&deviceProp, deviceId);
    fmt::println("Device {}: {}", deviceId, deviceProp.name);
    fmt::println("Imagen: {}x{} ({} bytes)", WIDTH, HEIGHT, buffer_size);

    uint8_t *d_src = nullptr;
    uint8_t *d_dst = nullptr;

    CHECK(cudaMalloc(&d_src, buffer_size));
    CHECK(cudaMalloc(&d_dst, buffer_size));
    CHECK(cudaMemcpy(d_src, original_buffer, buffer_size, cudaMemcpyHostToDevice));

    copiar_pesos(pesos.data(), (int)pesos.size());

    int thread_count = 1;
#pragma omp parallel
    {
#pragma omp master
        thread_count = omp_get_num_threads();
    }

    runtime_type modo = runtime_type::OPENMP;
    bool blur_aplicado = false;
    float tiempo_ms = 0.0f;

    sf::Vector2u windowSize = {WIDTH, HEIGHT};

    sf::RenderWindow window(sf::VideoMode(windowSize), "Gaussian Blur - SFML");

#ifdef _WIN32
    HWND hwnd = window.getNativeHandle();
    ShowWindow(hwnd, SW_MAXIMIZE);
#endif

    sf::Texture texture(windowSize);
    texture.update((const uint8_t *)display_buffer);
    sf::Sprite sprite(texture);

    sf::Font font("arial.ttf");
    sf::Text text(font, "Gaussian Blur", 22);
    text.setFillColor(sf::Color::White);
    text.setPosition({10, 10});
    text.setStyle(sf::Text::Bold);

    std::string options = "Controles: [B] Aplicar Blur | [R] Restaurar | [1] OpenMP | [2] CUDA";
    sf::Text textOptions(font, options, 18);
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
                case sf::Keyboard::Scan::B:
                {
                    sf::Clock timer; // medir tiempo de procesamiento

                    if (modo == runtime_type::OPENMP)
                    {
                        blur_openmp_regiones(original_buffer, display_buffer,
                                             WIDTH, HEIGHT, K, pesos);
                    }
                    else // runtime_type::CUDA
                    {
                        blur_gpu(d_src, d_dst, WIDTH, HEIGHT, K);
                        CHECK(cudaGetLastError());
                        CHECK(cudaMemcpy(display_buffer, d_dst, buffer_size, cudaMemcpyDeviceToHost));
                    }

                    tiempo_ms = timer.getElapsedTime().asSeconds() * 1000.0f;
                    blur_aplicado = true;
                    texture.update((const uint8_t *)display_buffer);
                    break;
                }
                case sf::Keyboard::Scan::R:
                    std::memcpy(display_buffer, original_buffer, buffer_size);
                    texture.update((const uint8_t *)display_buffer);
                    blur_aplicado = false;
                    tiempo_ms = 0.0f;
                    break;
                case sf::Keyboard::Scan::Num1:
                    modo = runtime_type::OPENMP;
                    break;
                case sf::Keyboard::Scan::Num2:
                    modo = runtime_type::CUDA;
                    break;
                default:
                    break;
                }
            }
        }

        frames++;
        if (clock.getElapsedTime().asSeconds() >= 1.0f)
        {
            fps = frames;
            frames = 0;
            clock.restart();
        }

        std::string mode_str = (modo == runtime_type::OPENMP)
                                   ? fmt::format("OpenMP ({} hilos)", thread_count)
                                   : fmt::format("CUDA ({})", deviceProp.name);

        auto msg = fmt::format(
            "Impl: {} | Estado: {} | Tiempo: {:.3f} ms | Imagen: {}x{} | FPS: {}",
            mode_str,
            blur_aplicado ? "BLUR aplicado" : "Original",
            tiempo_ms, WIDTH, HEIGHT, fps);

        text.setString(msg);

        window.clear();
        {
            window.draw(sprite);
            window.draw(text);
            window.draw(textOptions);
        }
        window.display();
    }

    CHECK(cudaFree(d_src));
    CHECK(cudaFree(d_dst));
    delete[] original_buffer;
    delete[] display_buffer;

    return 0;
}
