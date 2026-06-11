#include <cmath>
#include <fmt/core.h>
#include <iostream>
#include <mpi.h>
#include <vector>

#define MATRIX_DIM 25

void imprimir_matriz(const std::vector<double> &A, int rows, int cols)
{
  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < cols; j++)
    {
      fmt::print("{:.2f} ", A[i * cols + j]);
    }
    fmt::print("\n");
  }
}

void imprimir_vector(const std::vector<double> &b)
{
  fmt::println("{} : [", b.size());

  for (int i = 0; i < b.size(); i++)
  {
    fmt::print("{} ", b[i]);
  }
  fmt::print("]\n");
}

void multiplicar_matriz_vector(std::vector<double> &A, std::vector<double> &b,
                               std::vector<double> &x, int rows, int cols)
{
  for (int i = 0; i < rows; i++)
  {
    double sum = 0;
    for (int j = 0; j < cols; j++)
    {
      int index = i * cols + j;
      sum += A[index] * b[j];
    }
    x[i] = sum;
  }
}

int main(int argc, char **argv)
{
  MPI_Init(&argc, &argv);

  int nprocs;
  int rank;

  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int rows_per_rank = std::ceil(MATRIX_DIM * 1.0 / nprocs);
  int padding = rows_per_rank * nprocs - MATRIX_DIM;
  int padded_rows = MATRIX_DIM + padding;

  if (rank == 0)
  {
    fmt::println("nprocs: {}, rows_per_rank: {}, padding: {}, padded_rows: {}",
                 nprocs, rows_per_rank, padding, padded_rows);
  }

  std::vector<double> A;
  std::vector<double> b(MATRIX_DIM);
  std::vector<double> x;

  if (rank == 0)
  {
    A.resize(padded_rows * MATRIX_DIM, 0.0);
    x.resize(padded_rows);

    for (int i = 0; i < MATRIX_DIM; i++)
    {
      for (int j = 0; j < MATRIX_DIM; j++)
      {
        int index = i * MATRIX_DIM + j;
        A[index] = i;
      }
    }

    for (int i = 0; i < MATRIX_DIM; i++)
    {
      b[i] = 1;
    }
  }

  MPI_Bcast(b.data(), MATRIX_DIM, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  std::vector<double> A_local(rows_per_rank * MATRIX_DIM);
  std::vector<double> x_local(rows_per_rank);

  MPI_Scatter(A.empty() ? nullptr : A.data(), rows_per_rank * MATRIX_DIM, MPI_DOUBLE,
              A_local.data(), rows_per_rank * MATRIX_DIM, MPI_DOUBLE,
              0, MPI_COMM_WORLD);

  multiplicar_matriz_vector(A_local, b, x_local, rows_per_rank, MATRIX_DIM);

  MPI_Gather(x_local.data(), rows_per_rank, MPI_DOUBLE,
             x.empty() ? nullptr : x.data(), rows_per_rank, MPI_DOUBLE,
             0, MPI_COMM_WORLD);

  if (rank == 0)
  {
    x.resize(MATRIX_DIM);
    imprimir_vector(x);
  }

  MPI_Finalize();

  return 0;
}
