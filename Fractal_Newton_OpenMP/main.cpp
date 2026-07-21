#include <fmt/core.h>
#include <SFML/Graphics.hpp>
#include <complex>

#include "newton_serial.h"
#include "newton_openmp.h"
#include "palette.h"

#ifdef _WIN32
#include <windows.h>
#include <omp.h>
#endif

#define WIDTH 1600
#define HEIGHT 900

int max_iteraciones = 50;

double x_min = -1.5;
double x_max = 1.5;
double y_min = -1.0;
double y_max = 1.0;

uint32_t *pixel_buffer = nullptr; // entero sin signo de 32 bits

enum class runtime_type
{
    SERIAL = 0,
    OPENMP_REGIONES,
    OPENMP_FOR
};

int main()
{
    int thread_count;

#pragma omp parallel
    {
#pragma omp master
        thread_count = omp_get_num_threads();
    }

    runtime_type r_type = runtime_type::SERIAL;

    pixel_buffer = new uint32_t[WIDTH * HEIGHT];
    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Newton Fractal - OpenMP");

#ifdef _WIN32
    HWND hwnd = window.getNativeHandle();
    ShowWindow(hwnd, SW_MAXIMIZE);
#endif

    sf::Texture texture({WIDTH, HEIGHT});
    sf::Sprite sprite(texture);

    sf::Font font("arial.ttf");
    sf::Text text(font, "Newton Fractal", 24);
    text.setFillColor(sf::Color::White);
    text.setPosition({10, 10});
    text.setStyle(sf::Text::Bold);

    std::string options = "Opciones: [1] Serial | [2] OpenMP Regiones | [3] OpenMP For | Up/Down: Cambiar iteraciones";
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
                    r_type = runtime_type::SERIAL;
                    break;
                case sf::Keyboard::Scan::Num2:
                    r_type = runtime_type::OPENMP_REGIONES;
                    break;
                case sf::Keyboard::Scan::Num3:
                    r_type = runtime_type::OPENMP_FOR;
                    break;
                default:
                    break;
                }
            }
        }

        std::string mode = "";

        sf::Clock reloj_computo;

        if (r_type == runtime_type::SERIAL)
        {
            newton_serial(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, pixel_buffer);
            mode = "SERIAL";
        }
        else if (r_type == runtime_type::OPENMP_REGIONES)
        {
            newton_openmp_regiones(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, pixel_buffer);
            mode = fmt::format("OPENMP REGIONES ({} hilos)", thread_count);
        }
        else if (r_type == runtime_type::OPENMP_FOR)
        {
            newton_openmp_for(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, pixel_buffer);
            mode = fmt::format("OPENMP FOR ({} hilos)", thread_count);
        }

        double compute_ms = reloj_computo.getElapsedTime().asSeconds() * 1000.0;

                texture.update((const uint8_t *)pixel_buffer);

        // contar FPS
        frames++;

        if (clock.getElapsedTime().asSeconds() >= 1.0f)
        {
            fps = frames;
            frames = 0;
            clock.restart();
        }

        auto msg = fmt::format("Newton: iteraciones {}. FPS: {}. compute_ms: {:.3f}. Modo: {}",
                               max_iteraciones, fps, compute_ms, mode);
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
