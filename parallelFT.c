/**
 * (C) Copyright 2020
 */

#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <pthread.h>
#include <math.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

int N;
int numThreads;
double *values;
double complex *res;
double PI;
FILE *in;
FILE *out;

// Functie helper cu cateva prelucrari.
void getArgs(int argc, char **argv) {
    if (argc < 4) {
        printf("Not enough parameters!\nPlease use: "
               "./homeworkFT inputValues.txt outputValues.txt numThreads\n");
        exit(1);
    }

    in = fopen(argv[1], "r");
    out = fopen(argv[2], "w");
    numThreads = atoi(argv[3]);

    if (in == NULL || out == NULL) {
        printf("Error opening the files!\n");
        exit(2);
    }
}

// Functie de eliberare a memoriei.
void clean() {
    free(values);
    free(res);
    fclose(in);
    fclose(out);
}

// Functie de initializare si citire din fisier a datelor.
void init() {
    int i;

    PI = atan2(1, 1) * 4;

    if (fscanf(in, "%d", &N) == EOF) {
        printf("Failed to read n.\n");
        exit(0);
    }

    values = (double *) malloc(sizeof(double) * N);

    if (values == NULL) {
        printf("Malloc error!\n");
        exit(3);
    }

    res = (double complex *) malloc(sizeof(double complex) * N);

    if (res == NULL) {
        printf("Malloc error!\n");
        exit(3);
    }

    for (i = 0; i < N; ++i) {
        if (fscanf(in, "%lf", &values[i]) == EOF) {
            printf("Failed to read values[%d].\n", i);
            clean();
            exit(0);
        }
    }
}

// Functie helper cu cateva prelucrari.
void thread_info_init(pthread_t **tid, int **thread_id) {
    *tid = (pthread_t *) malloc(sizeof(pthread_t) * numThreads);

    if (*tid == NULL) {
        printf("Malloc error!\n");
        clean();
        exit(3);
    }

    *thread_id = (int *) malloc(sizeof(int) * numThreads);

    if (*thread_id == NULL) {
        printf("Malloc error!\n");
        free(*tid);
        clean();
        exit(3);
    }
}

// Functie de eliberare a memoriei vectorilor legati de threaduri.
void clean_thread_info(pthread_t **tid, int **thread_id) {
    free(*tid);
    free(*thread_id);
}

// Afisarea rezultatelor finale.
void show() {
    int i;

    fprintf(out, "%d\n", N);

    for (i = 0; i < N; ++i) {
        fprintf(out, "%lf %lf\n", creal(res[i]), cimag(res[i]));
    }
}

// Functia de rulat paralel - Transformata Fourier Discreta
void *fourier_transform(void *var) {
    int k, n;
    double complex sum;
    int thread_id = *(int*) var;
    int start = thread_id * ceil((double) N / numThreads);
    int end = MIN(N, (thread_id + 1) * ceil((double) N / numThreads));

    for (k  = start; k < end; ++k) {
        sum = 0;

        for (n  = 0; n < N; ++n) {
            sum += values[n] * cexp(-2 * PI * I * k * n / N);
        }

        res[k] = sum;
    }

    return NULL;
}

int main(int argc, char **argv) {
    int i;
    int *thread_id;
    pthread_t *tid;

    getArgs(argc, argv);
    init();
    thread_info_init(&tid, &thread_id);

    for (i = 0; i < numThreads; ++i) {
        thread_id[i] = i;
        pthread_create(&(tid[i]), NULL, fourier_transform, &(thread_id[i]));
    }

    for (i = 0; i < numThreads; ++i) {
        pthread_join(tid[i], NULL);
    }

    show();
    clean_thread_info(&tid, &thread_id);
    clean();
    return 0;
}
