#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define ROWS 10000
#define COLS 10000

int main(int argc, char **argv) {
    int *matrix;
    int size, proceso;
    int total_count = 0, partial_count = 0;
    int num_to_find;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &proceso);

    if (size != 4) {
        if (proceso == 0) {
            printf("Este programa debe ejecutarse con exactamente 4 procesadores.\n");
        }
        MPI_Finalize();
        return 1;
    }

    if (proceso == 0) {
        // Inicializar la matriz con valores aleatorios
        matrix = (int *) malloc(ROWS * COLS * sizeof(int));
        for (int i = 0; i < ROWS * COLS; i++) {
            matrix[i] = rand() % 100;
        }
    
        // Imprimir la matriz
        printf("Matriz:\n");
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                printf("%d ", matrix[i * COLS + j]);
            }
            printf("\n");
        }

        // Pedir al usuario que ingrese el número a buscar
        printf("\n");
        printf("===============================================\n");
        printf("Ingrese un número para buscar en la matriz: ");
        fflush(stdout);
        scanf("%d", &num_to_find);        
        printf("===============================================\n");

        // Enviar porciones de la matriz a cada proceso
        int rows_per_proc = ROWS / size;
        int extra_rows = ROWS % size;
        int send_count, send_start;
        for (int dest = 1; dest < size; dest++) {
            send_count = rows_per_proc * COLS;
            send_start = dest * rows_per_proc * COLS;
            if (dest <= extra_rows) {
                send_count += COLS;
                send_start += dest * COLS;
            } else {
                send_start += extra_rows * COLS;
            }
            MPI_Send(&matrix[send_start], send_count, MPI_INT, dest, 0, MPI_COMM_WORLD);
        }

        
        // Contar cuántas veces aparece el número deseado en la porción de la matriz del proceso 0
        for (int i = 0; i < rows_per_proc; i++) {
            for (int j = 0; j < COLS; j++) {
                if (matrix[i * COLS + j] == num_to_find) {
                    total_count++;
                }
            }
        }
    } else {
        // Recibir porción de la matriz del proceso 0
        int rows_per_proc = ROWS / size;
        int extra_rows = ROWS % size;
        int recv_count, recv_start;
        if (proceso <= extra_rows) {
            recv_count = rows_per_proc * COLS + COLS;
            recv_start = proceso * rows_per_proc * COLS;
        } else {
            recv_count = rows_per_proc * COLS;
            recv_start = proceso * rows_per_proc * COLS + extra_rows * COLS;
        }
        matrix = (int *) malloc(recv_count * sizeof(int));
        MPI_Recv(matrix, recv_count, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Cada proceso cuenta cuántas veces aparece el número deseado en su porción de la matriz
        for (int i = 0; i < rows_per_proc; i++) {
            for (int j = 0; j < COLS; j++) {
                if (matrix[i * COLS + j] == num_to_find) {
                    partial_count++;
                }
            }
        }
        // Enviar el resultado parcial al proceso 0
        MPI_Send(&partial_count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

if (proceso == 0) {
    // Recibir resultados parciales de los demás procesos y sumarlos
    for (int source = 1; source < size; source++) {
        int recv_count;
        MPI_Recv(&recv_count, 1, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        total_count += recv_count;
    }

    printf("El número %d aparece %d veces en la matriz\n", num_to_find, total_count);
    printf("===============================================\n");
    free(matrix);
}

MPI_Finalize();

return 0;
}