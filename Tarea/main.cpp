#include <iostream>
#include <vector>
#include <cmath>
#include <optional>

#include <omp.h>
#include <immintrin.h>

#include <fmt/core.h>
#include <fmt/ranges.h>
#include <SFML/Graphics.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

// Implementación de STB Image
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

enum class runtime_type
{
    ORIGINAL = 0,
    SIMD,
    OPENMP
};

void grayscale_simd(const uint8_t *rgba, uint8_t *gray, int num_pixels)
{
    int tope = (num_pixels / 8) * 8;

    __m256 w_r = _mm256_set1_ps(0.21f);
    __m256 w_g = _mm256_set1_ps(0.72f);
    __m256 w_b = _mm256_set1_ps(0.07f);

    for (int i = 0; i < tope; i += 8)
    {
        float r_arr[8], g_arr[8], b_arr[8];

        for (int j = 0; j < 8; j++)
        {
            int idx = (i + j) * 4;
            r_arr[j] = rgba[idx];
            g_arr[j] = rgba[idx + 1];
            b_arr[j] = rgba[idx + 2];
        }

        __m256 r = _mm256_loadu_ps(r_arr);
        __m256 g = _mm256_loadu_ps(g_arr);
        __m256 b = _mm256_loadu_ps(b_arr);

        __m256 r_mul = _mm256_mul_ps(r, w_r);
        __m256 g_mul = _mm256_mul_ps(g, w_g);
        __m256 b_mul = _mm256_mul_ps(b, w_b);

        __m256 sum = _mm256_add_ps(r_mul, _mm256_add_ps(g_mul, b_mul));

        float sum_arr[8];
        _mm256_storeu_ps(sum_arr, sum);

        for (int j = 0; j < 8; j++)
        {
            gray[i + j] = static_cast<uint8_t>(sum_arr[j]);
        }
    }

    for (int i = tope; i < num_pixels; i++)
    {
        int idx = i * 4;
        gray[i] = static_cast<uint8_t>(0.21f * rgba[idx] + 0.72f * rgba[idx + 1] + 0.07f * rgba[idx + 2]);
    }
}

void grayscale_openmp(const uint8_t *rgba, uint8_t *gray, int num_pixels)
{
#pragma omp parallel shared(rgba, gray, num_pixels)
    {
        int thread_id = omp_get_thread_num();
        int thread_count = omp_get_num_threads();

        int block_size = std::ceil(1.0 * num_pixels / thread_count);

        int start = thread_id * block_size;
        int end = (thread_id + 1) * block_size;

        if (end > num_pixels)
        {
            end = num_pixels;
        }

        for (int i = start; i < end; i++)
        {
            int idx = i * 4;
            gray[i] = static_cast<uint8_t>(0.21f * rgba[idx] + 0.72f * rgba[idx + 1] + 0.07f * rgba[idx + 2]);
        }
    }
}

void map_gray_to_rgba(const uint8_t *gray, uint8_t *display_rgba, int num_pixels)
{
#pragma omp parallel for
    for (int i = 0; i < num_pixels; i++)
    {
        int idx = i * 4;
        uint8_t val = gray[i];
        display_rgba[idx] = val;
        display_rgba[idx + 1] = val;
        display_rgba[idx + 2] = val;
        display_rgba[idx + 3] = 255;
    }
}

int main()
{
    int width, height, channels;
    uint8_t *rgba_pixels = stbi_load("aucas.png", &width, &height, &channels, STBI_rgb_alpha);
    channels = 4;
    int num_pixels = width * height;

    uint8_t *gray_pixels = new uint8_t[num_pixels];
    uint8_t *display_buffer = new uint8_t[num_pixels * 4];

    int thread_count;
#pragma omp parallel
    {
#pragma omp master
        thread_count = omp_get_num_threads();
    }

    runtime_type r_type = runtime_type::ORIGINAL;

    sf::RenderWindow window(sf::VideoMode({(unsigned int)width, (unsigned int)height}), "Filtro Escala de Grises - SFML");

#ifdef _WIN32
    HWND hwnd = window.getNativeHandle();
    ShowWindow(hwnd, SW_MAXIMIZE);
#endif

    sf::Texture texture({(unsigned int)width, (unsigned int)height});
    sf::Sprite sprite(texture);

    sf::Font font("arial.ttf");
    sf::Text text(font, "Escala de Grises", 24);
    text.setFillColor(sf::Color::Red);
    text.setPosition({10, 10});
    text.setStyle(sf::Text::Bold);

    std::string options = "Controles: [1] Original | [2] SIMD | [3] OpenMP | [S] Guardar Imagen";
    sf::Text textOptions(font, options, 20);
    textOptions.setFillColor(sf::Color::Red);
    textOptions.setStyle(sf::Text::Bold);
    textOptions.setPosition({10, window.getView().getSize().y - 40});

    int frames = 0;
    int fps = 0;
    sf::Clock clock;

    bool frame_needs_update = true;

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
                case sf::Keyboard::Scan::Num1:
                    r_type = runtime_type::ORIGINAL;
                    frame_needs_update = true;
                    break;
                case sf::Keyboard::Scan::Num2:
                    r_type = runtime_type::SIMD;
                    frame_needs_update = true;
                    break;
                case sf::Keyboard::Scan::Num3:
                    r_type = runtime_type::OPENMP;
                    frame_needs_update = true;
                    break;
                case sf::Keyboard::Scan::S:
                    if (r_type == runtime_type::SIMD)
                    {
                        stbi_write_png("salida_simd.png", width, height, STBI_grey, gray_pixels, width);
                        fmt::println("Imagen guardada: salida_simd.png");
                    }
                    else if (r_type == runtime_type::OPENMP)
                    {
                        stbi_write_png("salida_openmp.png", width, height, STBI_grey, gray_pixels, width);
                        fmt::println("Imagen guardada: salida_openmp.png");
                    }
                    break;
                }
            }
        }

        std::string mode = "";

        if (frame_needs_update)
        {
            if (r_type == runtime_type::ORIGINAL)
            {
                memcpy(display_buffer, rgba_pixels, num_pixels * 4);
                mode = "ORIGINAL";
            }
            else if (r_type == runtime_type::SIMD)
            {
                grayscale_simd(rgba_pixels, gray_pixels, num_pixels);
                map_gray_to_rgba(gray_pixels, display_buffer, num_pixels);
                mode = "SIMD";
            }
            else if (r_type == runtime_type::OPENMP)
            {
                grayscale_openmp(rgba_pixels, gray_pixels, num_pixels);
                map_gray_to_rgba(gray_pixels, display_buffer, num_pixels);
                mode = fmt::format("OPENMP ({} hilos)", thread_count);
            }

            texture.update((const uint8_t *)display_buffer);
            frame_needs_update = false;
        }
        else
        {
            if (r_type == runtime_type::ORIGINAL)
                mode = "ORIGINAL";
            else if (r_type == runtime_type::SIMD)
                mode = "SIMD";
            else if (r_type == runtime_type::OPENMP)
                mode = fmt::format("OPENMP ({} hilos)", thread_count);
        }

        frames++;
        if (clock.getElapsedTime().asSeconds() >= 1.0f)
        {
            fps = frames;
            frames = 0;
            clock.restart();
        }

        auto msg = fmt::format("Filtro: {}. FPS: {}", mode, fps);
        text.setString(msg);

        window.clear();
        window.draw(sprite);
        window.draw(text);
        window.draw(textOptions);
        window.display();
    }

    stbi_image_free(rgba_pixels);
    delete[] gray_pixels;
    delete[] display_buffer;

    return 0;
}