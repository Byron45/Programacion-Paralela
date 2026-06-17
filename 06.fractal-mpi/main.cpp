#include <mpi.h>
#include <fmt/core.h>
#include <complex>
#include <iostream>
#include "fractal_mpi.h"
#include <SFML/Graphics.hpp>
#include "draw_text.h"

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
int max_iteraciones = 10;

std::complex<double> c(-0.7, 0.27015);

uint32_t *pixel_buffer = nullptr;
uint32_t *texture_buffer = nullptr; // solo RANK_0

int running = 1;

int row_start;
int row_end;
int padding;
int delta;
int nprocs;
int rank;

//-- dimension imagen
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
        WIDTH, delta, texto.c_str(), 10, 25, 20);
}

void setup_ui()
{
    texture_buffer = new uint32_t[WIDTH * HEIGHT];
    std::memset(texture_buffer, 0, WIDTH * HEIGHT * sizeof(uint32_t));
    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Julia Set - SFML");

#ifdef _WIN32
    HWND hwnd = window.getNativeHandle();
    ShowWindow(hwnd, SW_MAXIMIZE);
#endif

    sf::Texture texture({WIDTH, HEIGHT});
    texture.update((const uint8_t *)texture_buffer);
    sf::Sprite sprite(texture);

    // textos
    const sf::Font font(arial_ttf::data, arial_ttf::data_len);
    sf::Text text(font, "Fractal", 24);
    text.setFillColor(sf::Color::White);
    text.setPosition({10, 10});
    text.setStyle(sf::Text::Bold);

    std::string options = "Up/Down: Change iteractions";
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
        // Process events
        while (const std::optional event = window.pollEvent())
        {
            // Close window: exit
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
                    {
                    }
                    break;
                default:
                    break;
                }
                std::memset(texture_buffer, 0, WIDTH * HEIGHT * sizeof(uint32_t));
            }
        }

        // Notificar a los otros RANKS que la app se esta cerrando
        std::vector<int> dummy = {max_iteraciones, running};
        MPI_Bcast(dummy.data(), 2, MPI_INT, 0, MPI_COMM_WORLD);

        if (running == 0)
        {
            break;
        }

        // dibujar la porcion del RANK_0
        julia_mpi(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, row_start, row_end, pixel_buffer);
        dibujar_texto(0);

        // copiar el pixel buffer a texture buffer
        std::memcpy(texture_buffer, pixel_buffer, WIDTH * delta * sizeof(uint32_t));

        // recibir las porciones
        for (int i = 1; i < nprocs; i++)
        {
            int new_delta = delta; // 225
            if (i == nprocs - 1)
            {
                new_delta = delta - padding;
            }

            MPI_Status status;

            MPI_Recv(
                pixel_buffer,
                WIDTH * delta,
                MPI_UNSIGNED,
                i, // rank origen
                0, // tag
                MPI_COMM_WORLD,
                &status);

            std::memcpy(texture_buffer + i * delta * WIDTH, pixel_buffer, WIDTH * new_delta * sizeof(uint32_t));
        }

        // Crear textura
        texture.update((const uint8_t *)texture_buffer);

        // contar fps
        frames++;
        if (clock.getElapsedTime().asSeconds() >= 1.0f)
        {
            fps = frames;
            frames = 0;
            clock.restart();
        }

        // actualizar el titulo de la ventana con el fps
        auto msg = fmt::format("Fractal: Iterations: {}, FPS: {}, Mode: MPI", max_iteraciones, fps);
        text.setString(msg);

        // Clear screen
        window.clear();
        {
            window.draw(sprite);
            window.draw(text);
            window.draw(textOptions);
        }

        // Update the window
        window.display();
    }
}

int main(int argc, char **argv)
{

    MPI_Init(&argc, &argv);

    nprocs;
    rank;

    init_freetype();

    // Ranks
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    delta = std::ceil((HEIGHT * 1.0) / nprocs); // 1600/4 = 400
    /**
     * r0: start = 0*400, end=0+400 = 400
     * r1: start = 1*400, end=400+400 = 800
     * r2: start = 2*400, end=800+400 = 1200
     * r3: start = 3*400, end=1200+400 = 1600
     */
    row_start = rank * delta;
    row_end = row_start + delta;
    padding = delta * nprocs - HEIGHT;

    if (row_end > HEIGHT)
    {
        row_end = HEIGHT;
    }

    pixel_buffer = new uint32_t[WIDTH * delta];
    std::memset(pixel_buffer, 0, WIDTH * delta * sizeof(uint32_t));
    fmt::println("RANK_{}: rows {} to {}", rank, row_start, row_end);
    if (rank == 0)
    {
        setup_ui();
    }
    else
    {
        // dibujar

        while (true)
        {
            std::vector<int> dummy = {max_iteraciones, 0};
            MPI_Bcast(dummy.data(), 2, MPI_INT, 0, MPI_COMM_WORLD);

            max_iteraciones = dummy[0];
            running = dummy[1];

            if (running == 0)
            {
                fmt::println("RANK_{}: received shutdown signal, exiting...", rank);
                break;
            }

            julia_mpi(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, row_start, row_end, pixel_buffer);
            dibujar_texto(rank);

            // neviar la porcion
            MPI_Send(
                pixel_buffer,
                WIDTH * delta,
                MPI_UNSIGNED,
                0, // rank origen
                0, // tag
                MPI_COMM_WORLD);

            /*if (rank == 1)
            {
                fmt::println("RANK_{}: max_iteraciones = {}", rank, max_iteraciones);
                std::cout.flush();
            }*/
        }
    }

    MPI_Finalize();
    return 0;
}