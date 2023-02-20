#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

#define ROWS 1000
#define COLS 1000

int main(int argc, char** argv) {

    int preceso, size;
    int matrix[ROWS][COLS];
    int diag1_sum = 0, diag1_max = 0;
    int diag2_sum = 0, diag2_max = 0;
    int max_diagonal, max_diagonal_id;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &preceso);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 5) {
        if (preceso == 0) {
            printf("El programa debe ejecutarse con 5 procesos.\n");
        }
        MPI_Finalize();
        exit(1);
    }

    if (preceso == 0) {
        // Generamos la matriz de números aleatorios
        srand(time(NULL)); // Inicializamos la semilla para obtener resultados reproducibles
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                matrix[i][j] = rand() % 99 + 1;
            }
        }

        // Imprimimos la matriz generada
        printf("Matriz generada:\n");
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                printf("%d ", matrix[i][j]);
            }
            printf("\n");
        }
    }

    // Enviamos la matriz a todos los procesos
    MPI_Bcast(matrix, ROWS*COLS, MPI_INT, 0, MPI_COMM_WORLD);

    // Procesamos la diagonal principal y secundaria
    for (int i = 0; i < ROWS; i++) {
        if (i % size == 0) {
            // Procesador 0: extrae la diagonal principal y secundaria
            diag1_sum += matrix[i][i];
            if (matrix[i][i] > diag1_max) {
                diag1_max = matrix[i][i];
            }

            diag2_sum += matrix[i][COLS-i-1];
            if (matrix[i][COLS-i-1] > diag2_max) {
                diag2_max = matrix[i][COLS-i-1];
            }
        } else if (i % size == 1 && preceso == 1) {
            // Procesador 1: calcula la suma de la diagonal principal
            MPI_Reduce(&diag1_sum, &diag1_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        } else if (i % size == 2 && preceso == 2) {
            // Procesador 2: calcula el valor máximo de la diagonal principal
            MPI_Reduce(&diag1_max, &diag1_max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
        } else if (i % size == 3 && preceso == 3) {
            // Procesador 3: calcula la suma de la diagonal secundaria
            MPI_Reduce(&diag2_sum, &diag2_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        } else if (i % size == 4 && preceso == 4) {
            // Procesador 4: calcula el valor máximo de la diagonal secundaria
            MPI_Reduce(&diag2_max, &diag2_max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
        }
    
    }

    // Procesador 0: determina la diagonal con el valor máximo
    if (preceso == 0) {
        if (diag1_max > diag2_max) {
            max_diagonal = diag1_max;
            max_diagonal_id = 1;
        } else {
            max_diagonal = diag2_max;
            max_diagonal_id = 2;
        }
    }

    // Enviamos el resultado de la diagonal con el valor máximo a todos los procesos
    MPI_Bcast(&max_diagonal_id, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Procesador 0: determina qué diagonal es más grande
    if (preceso == 0) {
        printf("================================================\n");
        printf("Suma de la diagonal principal: %d\n", diag1_sum);    
        printf("Valor máximo de la diagonal principal: %d\n", diag1_max);
        printf("================================================\n");
        printf("Suma de la diagonal secundaria: %d\n", diag2_sum);
        printf("Valor máximo de la diagonal secundaria: %d\n", diag2_max);
        printf("================================================\n");
        printf("Valor máximo entre las dos diagonales: %d\n", max_diagonal);
        printf("================================================\n");
        if (diag1_sum > diag2_sum) {
            printf("La diagonal principal es la más grande\n");
        } else if (diag1_sum < diag2_sum)        {
            printf("La diagonal secundaria es la más grande\n");
        } else{
            printf("Ambas diagonales tienen el mismo tamaño\n");
        }
        printf("================================================\n");
    }

    MPI_Finalize();
    return 0;
}


