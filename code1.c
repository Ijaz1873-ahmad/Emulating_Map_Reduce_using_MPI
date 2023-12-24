

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

#define MATRIX_SIZE 16
#define NUM_MAPPERS 7

typedef struct {
    int row;
    int col;
    float val;
} MatrixEntry;

void read_matrix(const char *filename, float *matrix) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Could not open file %s\n", filename);
        exit(1);
    }

    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            fscanf(fp, "%f", &matrix[i * MATRIX_SIZE + j]);
        }
    }

    fclose(fp);
}

void map(MatrixEntry *input, MatrixEntry *output, int size) {
    for (int i = 0; i < size; i++) {
        int row = input[i].row;
        int col = input[i].col;
        float val = input[i].val;
        for (int j = 0; j < MATRIX_SIZE; j++) {
            output[i * MATRIX_SIZE + j].row = row;
            output[i * MATRIX_SIZE + j].col = j;
            output[i * MATRIX_SIZE + j].val = val;
        }
    }
}

void reduce(MatrixEntry *input, float *output, int size) {
    for (int i = 0; i < size; i++) {
        int row = input[i].row;
        int col = input[i].col;
        float val = input[i].val;
        output[row * MATRIX_SIZE + col] += val;
    }
}

int main(int argc, char *argv[]) {
    int num_procs, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int rows_per_mapper = MATRIX_SIZE / NUM_MAPPERS;
    int num_entries_per_mapper = rows_per_mapper * MATRIX_SIZE;
    int num_entries_per_reducer = rows_per_mapper * NUM_MAPPERS * MATRIX_SIZE;

    srand(time(NULL));
    int num_reducers = rand() % (num_procs - NUM_MAPPERS - 1) + 1;
    int *reducer_ranks = (int *)malloc(num_reducers * sizeof(int));
    for (int i = 0; i < num_reducers; i++) {
        reducer_ranks[i] = rand() % (num_procs - NUM_MAPPERS) + NUM_MAPPERS + 1;
    }

    if (rank == 0) {
        printf("Master with process_id 0 running on master\n");
        float matrix_a[MATRIX_SIZE * MATRIX_SIZE];
        
        float matrix_b[MATRIX_SIZE * MATRIX_SIZE];
        read_matrix("matrix_a.txt", matrix_a);
        read_matrix("matrix_b.txt", matrix_b);

        // Send matrix B to all mappers
        MPI_Bcast(matrix_b, MATRIX_SIZE * MATRIX_SIZE, MPI_FLOAT, 0, MPI_COMM_WORLD);

        // Send matrix A rows to each mapper
        for (int i = 0; i < NUM_MAPPERS; i++) {
                   MatrixEntry *entries = (MatrixEntry*)malloc(num_entries_per_mapper * sizeof(MatrixEntry));
        for (int j = 0; j < rows_per_mapper; j++) {
            for (int k = 0; k < MATRIX_SIZE; k++) {
                int index = j * MATRIX_SIZE + k;
                entries[index].row = i * rows_per_mapper + j;
                entries[index].col = k;
                entries[index].val = matrix_a[entries[index].row * MATRIX_SIZE + entries[index].col];
            }
        }
        MPI_Send(entries, num_entries_per_mapper, MPI_FLOAT_INT, i + 1, 0, MPI_COMM_WORLD);
        free(entries);
    }

    // Receive results from reducers
    float *result = (float*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(float));
    for (int i = 0; i < num_reducers; i++) {
        MPI_Recv(result + i * num_entries_per_reducer, num_entries_per_reducer, MPI_FLOAT, reducer_ranks[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Print result matrix
    printf("Result:\n");
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            printf("%f ", result[i * MATRIX_SIZE + j]);
        }
        printf("\n");
    }

    free(result);
} else if (rank >= 1 && rank <= NUM_MAPPERS) {
    // Mapper processes
    printf("Task map assigned to process %d\n", rank);
    MatrixEntry *entries = (MatrixEntry*)malloc(num_entries_per_mapper * sizeof(MatrixEntry));
    MPI_Recv(entries, num_entries_per_mapper, MPI_FLOAT_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // Map function
    MatrixEntry *output = (MatrixEntry*)malloc(num_entries_per_mapper * MATRIX_SIZE * sizeof(MatrixEntry));
    map(entries, output, num_entries_per_mapper);
    printf("Process %d received task map\n", rank);

    // Send results to reducers
    for (int i = 0; i < num_reducers; i++) {
        MPI_Send(output + i * num_entries_per_reducer, num_entries_per_reducer, MPI_FLOAT_INT, reducer_ranks[i], 0, MPI_COMM_WORLD);
    }
    printf("Process %d has completed task map\n", rank);
    free(entries);
    free(output);
} else {
    int is_reducer = 0;
    for (int i = 0; i < num_reducers; i++) {
        if (rank == reducer_ranks[i]) {
            is_reducer = 1;
            break;
        }
    }

    if (is_reducer) {
        // Reducer processes
        printf("Task reduce assigned to process %d\n", rank);
        MatrixEntry *entries = (MatrixEntry*)malloc(num_entries_per_reducer * sizeof(MatrixEntry));
        MPI_Recv(entries, num_entries_per_reducer, MPI_FLOAT_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Reduce function
        float *output = (float*)calloc(MATRIX_SIZE * MATRIX_SIZE, sizeof(float));
        reduce(entries, output, num_entries_per_reducer);
        printf("Process %d received task Reduce\n", rank);
        printf("Process %d has completed task Reduce\n", rank);

        // Send results to master
        MPI_Send(output, MATRIX_SIZE * MATRIX_SIZE, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);

           free(entries);
    free(output);
}

if (rank == 0) {
    printf("Job has been completed\n");
}

free(reducer_ranks);
MPI_Finalize();
return 0;
}




