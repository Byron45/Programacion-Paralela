#include <iostream>
#include <fmt/core.h>

int main()
{
    const char* valor = "xyz";
    std::printf("Hola, mundo: %s\n", valor);
    fmt::println("Hola, mundo con fmt: {}", valor);
    return 0;
}