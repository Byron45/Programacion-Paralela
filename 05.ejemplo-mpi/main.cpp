#include <iostream>
#include <mpi.h>
#include <fmt/core.h>

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int nprocs;
    int rank;

    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int version, subversion;
    MPI_Get_version(&version, &subversion);

    if (rank == 0)
    {
        fmt::println("MPI version: {}.{}", version, subversion);
        fmt::println("Número de procesos: {}", nprocs);
    }

    fmt::println("RANK{} de procesos: {}", rank, nprocs);

    // while (1)
    // {
    // }

    MPI_Finalize();
    return 0;
}