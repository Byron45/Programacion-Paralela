#include <iostream>
#include <fmt/core.h>
#include <omp.h>

int main()
{
    // #pragma omp parallel num_threads(4) //numero de hilos a usar
    //{
    // #pragma omp master
    //       {

    //        int threads_count = omp_get_num_threads();
    //        fmt::println("Hello serial world, goodbye OpenMP");
    //        fmt::println("I have {} thread(s)", threads_count);
    //    }

    //    int thread_id = omp_get_thread_num();

    //    fmt::println("My thread_id is {}", thread_id);
    //}

    // #pragma omp parallel
    //     {

    //         int thread_id = omp_get_thread_num();
    //         std::string msg = "";

    // #pragma omp parallel for
    //         {
    //             for (int i = 0; i < thread_id; i++)
    //             {
    //                 msg = msg + "*";
    //             }
    //         }

    //         fmt::println("My thread_id is {}, msg: {}", thread_id, msg);
    //     }

    int num_elementos = 15;
#pragma omp parallel for num_threads(4)
    for (int i = 0; i < num_elementos; i++)
    {
        // fmt::println("i: {}, thread_id: {}", i, omp_get_thread_num());
    }

#pragma omp parallel num_threads(4)
    {
        int thread_id = omp_get_thread_num();
        int thread_num = omp_get_num_threads();

        int delta = std::ceil(num_elementos * 1.0 / thread_num);
        int start = thread_id * delta;
        int end = (thread_id + 1) * delta;

        if (thread_id == thread_num - 1)
            end = num_elementos;

        fmt::println("Thread_{}: start: {}, end: {}", thread_id, start, end);

        for (int i = start; i < end; i++)
        {
        }
    }

    // Primos
#pragma omp parallel num_threads(4)
    {

        int thread_id = omp_get_thread_num();
        int thread_num = omp_get_num_threads();

        for (int i = thread_id; i < num_elementos; i += 4)
        {
            fmt::println("Thread_{}: index: {}", thread_id, i);
        }
    }

#pragma omp parallel
    {
        while (true)
        {
        }
    }
}