#include <cmath>
#include <fmt/core.h>
#include <iostream>
#include <mpi.h>
#include <vector>

#define MATRIX_DIM 25

void imprimir_matriz(const std::vector<double> &A, int rows, int cols) {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      fmt::print("{:.2f} ", A[i * cols + j]);
    }
    fmt::print("\n");
  }
}

void imprimir_vector(const std::vector<double> &b) {
  fmt::println("{} : [", b.size());

  for (int i = 0; i < b.size(); i++) {
    fmt::print("{} ", b[i]);
  }
  fmt::print("]\n");
}

void multiplicar_matriz_vector(std::vector<double> &A, std::vector<double> &b,
                               std::vector<double> &x, int rows, int cols) {
  for (int i = 0; i < rows; i++) {
    double sum = 0;
    for (int j = 0; j < cols; j++) {
      int index = i * cols + j;
      sum += A[index] * b[j];
    }
    x[i] = sum;
  }
}

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);

  int nprocs;
  int rank;

  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) {

    // Numero de filas fijas para CADA RANK (proceso)
    int rows_per_rank = std::ceil(MATRIX_DIM * 1.0 / nprocs);

    // Calculamos cuanto relleno necesitamos para que sea divisible exacto
    int padding = rows_per_rank * nprocs - MATRIX_DIM;
    int padded_rows =
        MATRIX_DIM + padding; // Total de filas con el relleno incluido

    fmt::println("nprocs: {}, rows_per_rank: {}, padding: {}, padded_rows: {}",
           nprocs, rows_per_rank, padding, padded_rows);

    std::vector<double> A(padded_rows * MATRIX_DIM, 0.0);
    std::vector<double> b(MATRIX_DIM);
    std::vector<double> x(padded_rows, 0.0);

    // inicializar la matriz A y el vector b
    for (int i = 0; i < MATRIX_DIM; i++) {
      for (int j = 0; j < MATRIX_DIM; j++) {
        int index = i * MATRIX_DIM + j;
        A[index] = i;
      }
    }

    for (int i = 0; i < MATRIX_DIM; i++) {
      b[i] = 1;
    }

    // Numero de filas para cada RANK (proceso)
    // int rows_per_rank = std::ceil(MATRIX_DIM * 1.0 / nprocs);
    // int padding = rows_per_rank * nprocs - MATRIX_DIM;

    // fmt::print("MATRIX_DIM: {}, nprocs: {}, rows_per_rank: {}, padding:
    // {}\n", MATRIX_DIM, nprocs, rows_per_rank, padding);

    // enviar dimensiones y datos
    for (int i = 1; i < nprocs; i++) {
      int filas = rows_per_rank;
      // if (i == nprocs - 1)
      // {
      //     filas = rows_per_rank - padding;
      // }

      // enviar dimension
      std::vector<int> data = {MATRIX_DIM, filas};

      MPI_Send(data.data(),     // buffer
               2,               // count
               MPI_INT,         // Tipo de datos
               i,               // RANK destino
               0,               // tag
               MPI_COMM_WORLD); // grupo

      // enviar datos al rank i
      const double *buffer = A.data();
      MPI_Send(&buffer[i * rows_per_rank * MATRIX_DIM], // buffer
               filas * MATRIX_DIM,                      // count
               MPI_DOUBLE,                              // Tipo de datos
               i,                                       // RANK destino
               0,                                       // tag
               MPI_COMM_WORLD);                         // grupo

      // enviar el vector b

      MPI_Send(b.data(),        // buffer
               MATRIX_DIM,      // count
               MPI_DOUBLE,      // Tipo de datos
               i,               // RANK destino
               0,               // tag
               MPI_COMM_WORLD); // grupo
    }

    // fmt::print("RANK_{}, {} x {}\n", rank, rows_per_rank, MATRIX_DIM);

    // multiplicar matriz por vector
    multiplicar_matriz_vector(A, b, x, rows_per_rank, MATRIX_DIM);

    // imprimir_vector(x_local);

    for (int i = 1; i < nprocs; i++) {
      int filas = rows_per_rank;
      // if (i == nprocs - 1)
      // {
      //     filas = rows_per_rank - padding;
      // }
      /*
      [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24]
      R0: [x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,.........]
      R1: [.................................................,y,y,y,y,y]
      R2:
      [.........................................................,z,z,z,z,z,z]
      R3:
      [.......................................................................,w,w,w,w,w,]
       */
      MPI_Recv(x.data() + i * rows_per_rank, // buffer
               filas,                        // count
               MPI_DOUBLE,                   // Tipo de datos
               i,                            // RANK origen
               0,                            // tag
               MPI_COMM_WORLD,               // grupo
               MPI_STATUS_IGNORE);           // status
    }

    x.resize(MATRIX_DIM); // Eliminar el relleno

    // fmt::print("RANK_{}, resultado:\n", rank);
    imprimir_vector(x);
  } else {
    std::vector<int> data_rec(2);
    MPI_Recv(data_rec.data(), // buffer
             2,               // count
             MPI_INT,         // Tipo de datos
             0,               // RANK origen
             0,               // tag
             MPI_COMM_WORLD,  // grupo
             MPI_STATUS_IGNORE);

    int matrix_dim = data_rec[0];
    int rows = data_rec[1];

    // fmt::print("Rank {}, {} x {}\n", rank, rows, matrix_dim);

    std::vector<double> A_local(rows * matrix_dim);
    std::vector<double> b_local(MATRIX_DIM);

    MPI_Recv(A_local.data(),    // buffer
             rows * matrix_dim, // count
             MPI_DOUBLE,        // Tipo de datos
             0,                 // RANK origen
             0,                 // tag
             MPI_COMM_WORLD,    // grupo
             MPI_STATUS_IGNORE);

    MPI_Recv(b_local.data(), // buffer
             MATRIX_DIM,     // count
             MPI_DOUBLE,     // Tipo de datos
             0,              // RANK origen
             0,              // tag
             MPI_COMM_WORLD, // grupo
             MPI_STATUS_IGNORE);

    if (rank == 2) {
      // imprimir_matriz(A_local, rows, matrix_dim);
      // fmt::print(" ");
      // imprimir_vector(b_local);
    }

    // multiplicar matriz por vector
    std::vector<double> x_local(rows);
    multiplicar_matriz_vector(A_local, b_local, x_local, rows, matrix_dim);

    // fmt::print("RANK_{}, resultado:\n", rank);
    // imprimir_vector(x_local);

    MPI_Send(x_local.data(),  // buffer
             rows,            // count
             MPI_DOUBLE,      // Tipo de datos
             0,               // RANK destino
             0,               // tag
             MPI_COMM_WORLD); // grupo
  }

  MPI_Finalize();

  return 0;
}