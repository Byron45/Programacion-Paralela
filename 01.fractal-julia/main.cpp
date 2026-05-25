#include <fmt/core.h>
#include <SFML/Graphics.hpp>
#include <complex>

#include "fractal_serial.h"
#include "fractal_simd.h"
#include "fractal_openmp.h"
#ifdef _WIN32
#include <windows.h>
#include <omp.h>
#endif

// Dimension de la imagen
#define WIDTH 1920
#define HEIGHT 1080

// Parametros
int max_iteraciones = 10;

double x_min = -1.5;
double x_max = 1.5;
double y_min = -1.0;
double y_max = 1.0;

std::complex<double> c(-0.7, 0.27015);

// Textura
uint32_t *pixel_buffer = nullptr; // entero sin signo de 32 bits

enum class runtime_type
{
    SERIAL_1 = 0,
    SERIAL_2,
    SIMD,
    OPENMP,
    OPENMPFOR,
    OPENMPFORSIMD
};

int main()

{

    int thread_count;

#pragma omp parallel
    {
#pragma omp master
        thread_count = omp_get_num_threads();
    }

    runtime_type r_type = runtime_type::SERIAL_1;

    pixel_buffer = new uint32_t[WIDTH * HEIGHT];
    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Julia Set - SFML");

#ifdef _WIN32
    HWND hwnd = window.getNativeHandle();
    ShowWindow(hwnd, SW_MAXIMIZE);
#endif

    sf::Texture texture({WIDTH, HEIGHT});
    sf::Sprite sprite(texture);

    sf::Font font("arial.ttf");
    sf::Text text(font, "Julia Set", 24);
    text.setFillColor(sf::Color::White);
    text.setPosition({10, 10});
    text.setStyle(sf::Text::Bold);

    std::string options = "Options: [1] Serial 1 [2] Serial 2 [3] SIMD | [4] OPENMP | [5] OPENMP FOR | [6] OPENMP FOR SIMD | Up/Down: Change Iterations";
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
                case sf::Keyboard::Scan::Num1:
                    r_type = runtime_type::SERIAL_1;
                    break;
                case sf::Keyboard::Scan::Num2:
                    r_type = runtime_type::SERIAL_2;
                    break;
                case sf::Keyboard::Scan::Num3:
                    r_type = runtime_type::SIMD;
                    break;
                case sf::Keyboard::Scan::Num4:
                    r_type = runtime_type::OPENMP;
                    break;
                case sf::Keyboard::Scan::Num5:
                    r_type = runtime_type::OPENMPFOR;
                    break;
                case sf::Keyboard::Scan::Num6:
                    r_type = runtime_type::OPENMPFORSIMD;
                    break;
                }
            }

        std:
            memset(pixel_buffer, 0, WIDTH * HEIGHT * sizeof(uint32_t));
        }

        std::string mode = "";
        if (r_type == runtime_type::SERIAL_1)
        {
            julia_serial_1(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, pixel_buffer);
            mode = "SERIAL_1";
        }
        else if (r_type == runtime_type::SERIAL_2)
        {
            julia_serial_2(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, pixel_buffer);
            mode = "SERIAL_2";
        }

        else if (r_type == runtime_type::SIMD)
        {
            julia_simd(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, pixel_buffer);
            mode = "SIMD";
        }

        else if (r_type == runtime_type::OPENMP)
        {
            julia_openmp_regiones(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, pixel_buffer);
            mode = fmt::format("Julia OPENMP  {} (threads)", thread_count);
            ;
        }

        else if (r_type == runtime_type::OPENMPFOR)
        {
            julia_openmp_for(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, pixel_buffer);
            mode = fmt::format("Julia OPENMP FOR  {} (threads)", thread_count);
        }

        else if (r_type == runtime_type::OPENMPFORSIMD)
        {
            julia_openmp_for_simd(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, pixel_buffer);
            mode = fmt::format("Julia OPENMP FOR SIMD  {} (threads)", thread_count);
        }

        // dibujar fractal
        // pintamos

        // crear la textura
        texture.update((const uint8_t *)pixel_buffer);

        // contar FPS
        frames++;

        if (clock.getElapsedTime().asSeconds() >= 1.0f)
        {
            fps = frames;
            frames = 0;
            clock.restart();
        }

        // actualizar el titulo de la ventana con el FPS
        auto msg = fmt::format("Julia Set: iterations {}. FPS: {}. Mode: {}", max_iteraciones, fps, mode);
        text.setString(msg);

        window.clear();

        {
            window.draw(sprite);
            window.draw(text);
            window.draw(textOptions);
        }

        window.display();
    }

    delete[] pixel_buffer;

    return 0;
}
