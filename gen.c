#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MATRIX_SIZE (1 << 4) // 2^4

void generate_random_matrix(int *matrix) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix[i * MATRIX_SIZE + j] = (rand() % 100) + 1;
        }
    }
}

void write_matrix_to_file(const char *filename, int *matrix) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("Could not open file %s\n", filename);
        exit(1);
    }

    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            fprintf(fp, "%d ", matrix[i * MATRIX_SIZE + j]);
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
}

int main() {
    srand(time(NULL));

    int matrix_a[MATRIX_SIZE * MATRIX_SIZE];
    int matrix_b[MATRIX_SIZE * MATRIX_SIZE];

    generate_random_matrix(matrix_a);
    generate_random_matrix(matrix_b);

    write_matrix_to_file("matrix_a.txt", matrix_a);
    write_matrix_to_file("matrix_b.txt", matrix_b);

    return 0;
}

