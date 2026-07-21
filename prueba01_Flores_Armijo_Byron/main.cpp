#include <mpi.h>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>

#include "newton_mpi.h"
#include "palette.h"
#include "draw_text.h"

#include <SFML/Graphics.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

namespace arial_ttf
{
    extern size_t data_len;
    extern unsigned char data[];
}

double x_min = -1.5;
double x_max = 1.5;
double y_min = -1.0;
double y_max = 1.0;
int max_iteraciones = 50;

uint32_t *pixel_buffer = nullptr;
uint32_t *texture_buffer = nullptr;

int running = 1;

int row_start;
int row_end;
int padding;
int delta;
int nprocs;
int rank;

#define WIDTH 1600
#define HEIGHT 900

std::string machine_name()
{
    std::string mname = "";
#ifdef _WIN32
    char hostname[256];
    DWORD size = sizeof(hostname);
    GetComputerNameA(hostname, &size);
    mname = hostname;
#endif
    return mname;
}

void dibujar_texto(int rank)
{
    auto texto = fmt::format("RANK_{} --> {}", rank, machine_name());

    draw_text_to_texture(
        (unsigned char *)pixel_buffer,
        WIDTH, delta, texto.c_str(), WIDTH - 320, 25, 20);
}

void setup_ui()
{
    texture_buffer = new uint32_t[WIDTH * HEIGHT];
    std::memset(texture_buffer, 0, WIDTH * HEIGHT * sizeof(uint32_t));

    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Newton Fractal - MPI - SFML");

#ifdef _WIN32
    HWND hwnd = window.getNativeHandle();
    ShowWindow(hwnd, SW_MAXIMIZE);
#endif

    sf::Texture texture({WIDTH, HEIGHT});
    texture.update((const uint8_t *)texture_buffer);
    sf::Sprite sprite(texture);

    const sf::Font font("arial.ttf");

    sf::Text text(font, "Newton Fractal", 18);
    text.setFillColor(sf::Color::White);
    text.setPosition({10, 10});
    text.setStyle(sf::Text::Bold);

    std::string options = "Controles: [Up/Down] Cambiar iteraciones | [Esc] Salir";
    sf::Text textOptions(font, options, 20);
    textOptions.setFillColor(sf::Color::White);
    textOptions.setStyle(sf::Text::Bold);
    textOptions.setPosition({10, window.getView().getSize().y - 40});

    int frames = 0;
    int fps = 0;
    sf::Clock clock;

    sf::Clock reloj_computo;

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                running = 0;
                window.close();
            }
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
                    running = 0;
                    window.close();
                    break;
                default:
                    break;
                }
            }
        }

        std::vector<int> control = {max_iteraciones, running};
        MPI_Bcast(control.data(), 2, MPI_INT, 0, MPI_COMM_WORLD);

        if (running == 0)
        {
            break;
        }

        std::vector<double> dominio = {x_min, x_max, y_min, y_max};
        MPI_Bcast(dominio.data(), 4, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        double local_total_iters = 0.0;
        reloj_computo.restart();
        newton_mpi(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, row_start, row_end,
                   max_iteraciones, pixel_buffer, &local_total_iters);
        double local_compute_ms = reloj_computo.getElapsedTime().asSeconds() * 1000.0;

        dibujar_texto(0);

        std::memcpy(texture_buffer, pixel_buffer, WIDTH * delta * sizeof(uint32_t));

        for (int i = 1; i < nprocs; i++)
        {
            int new_delta = delta;
            if (i == nprocs - 1)
            {
                new_delta = delta - padding;
            }

            MPI_Status status;
            MPI_Recv(
                pixel_buffer,
                WIDTH * delta,
                MPI_UNSIGNED,
                i,
                0,
                MPI_COMM_WORLD,
                &status);

            std::memcpy(texture_buffer + i * delta * WIDTH, pixel_buffer, WIDTH * new_delta * sizeof(uint32_t));
        }

        double max_compute_ms = 0.0;
        double total_iters = 0.0;

        MPI_Reduce(&local_compute_ms, &max_compute_ms, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
        MPI_Reduce(&local_total_iters, &total_iters, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

        texture.update((const uint8_t *)texture_buffer);

        frames++;
        if (clock.getElapsedTime().asSeconds() >= 1.0f)
        {
            fps = frames;
            frames = 0;
            clock.restart();
        }

        auto msg = fmt::format(
            "RANKs: 0-{} (nprocs={})\n"
            "Iteraciones: {}\n"
            "max_compute_ms: {:.3f}\n"
            "total_iters: {:.0f}\n"
            "FPS: {}",
            nprocs - 1, nprocs,
            max_iteraciones,
            max_compute_ms,
            total_iters,
            fps);
        text.setString(msg);

        window.clear();
        {
            window.draw(sprite);
            window.draw(text);
            window.draw(textOptions);
        }

        window.display();
    }
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    init_freetype();

    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    delta = std::ceil((HEIGHT * 1.0) / nprocs);
    row_start = rank * delta;
    row_end = row_start + delta;
    padding = delta * nprocs - HEIGHT;

    if (row_end > HEIGHT)
    {
        row_end = HEIGHT;
    }

    pixel_buffer = new uint32_t[WIDTH * delta];
    std::memset(pixel_buffer, 0, WIDTH * delta * sizeof(uint32_t));

    fmt::println("RANK_{}: filas {} a {}", rank, row_start, row_end);

    if (rank == 0)
    {
        setup_ui();
    }
    else
    {
        sf::Clock reloj_computo;

        while (true)
        {
            std::vector<int> control = {max_iteraciones, 0};
            MPI_Bcast(control.data(), 2, MPI_INT, 0, MPI_COMM_WORLD);

            max_iteraciones = control[0];
            running = control[1];

            if (running == 0)
            {
                fmt::println("RANK_{}: señal de apagado recibida, saliendo...", rank);
                break;
            }

            std::vector<double> dominio = {x_min, x_max, y_min, y_max};
            MPI_Bcast(dominio.data(), 4, MPI_DOUBLE, 0, MPI_COMM_WORLD);
            x_min = dominio[0];
            x_max = dominio[1];
            y_min = dominio[2];
            y_max = dominio[3];

            double local_total_iters = 0.0;
            reloj_computo.restart();
            newton_mpi(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, row_start, row_end,
                       max_iteraciones, pixel_buffer, &local_total_iters);
            double local_compute_ms = reloj_computo.getElapsedTime().asSeconds() * 1000.0;

            dibujar_texto(rank);

            MPI_Send(
                pixel_buffer,
                WIDTH * delta,
                MPI_UNSIGNED,
                0,
                0,
                MPI_COMM_WORLD);

            MPI_Reduce(&local_compute_ms, nullptr, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
            MPI_Reduce(&local_total_iters, nullptr, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        }
    }

    delete[] pixel_buffer;
    if (rank == 0)
    {
        delete[] texture_buffer;
    }

    MPI_Finalize();
    return 0;
}
