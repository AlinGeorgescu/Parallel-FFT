/**
 * (C) Copyright 2020
 */

#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <pthread.h>
#include <math.h>

int N;
int numThreads;
double complex *values;
double complex *helper;
double PI;
FILE *in;
FILE *out;

typedef struct {
    double complex *buf;
    double complex *out;
    int step;
} arg;

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
    free(helper);
    fclose(in);
    fclose(out);
}

// Functie de initializare si citire din fisier a datelor.
void init() {
    int i;
    double temp;

    PI = atan2(1, 1) * 4;

    if (fscanf(in, "%d", &N) == EOF) {
        printf("Failed to read n.\n");
        exit(0);
    }

    values = (double complex *) malloc(sizeof(double complex) * N);

    if (values == NULL) {
        printf("Malloc error!\n");
        exit(3);
    }

    helper = (double complex *) malloc(sizeof(double complex) * N);

    if (helper == NULL) {
        printf("Malloc error!\n");
        exit(3);
    }

    for (i = 0; i < N; ++i) {
        if (fscanf(in, "%lf", &temp) == EOF) {
            printf("Failed to read values[%d].\n", i);
            clean();
            exit(0);
        }

        values[i] = temp;
        helper[i] = temp;
    }
}

// Afisarea rezultatelor finale.
void show() {
    int i;

    fprintf(out, "%d\n", N);

    for (i = 0; i < N; ++i) {
        fprintf(out, "%lf %lf\n", creal(values[i]), cimag(values[i]));
    }
}

/**
 * Functia de rulat complet secvential
 * Transformata Fourier Rapida
 */
void full_seq_fft(double complex *buf, double complex *out, int step) {
    int i;
    double complex t;

    if (step < N) {
        full_seq_fft(out, buf, step * 2);
        full_seq_fft(out + step, buf + step, step * 2);
 
        for (i = 0; i < N; i += 2 * step) {
            t = cexp(-I * PI * i / N) * out[i + step];
            buf[i / 2] = out[i] + t;
            buf[(i + N) / 2] = out[i] - t;
        }
    }
}

/**
 * Functia de rulat in paralel
 * Transformata Fourier Rapida
 */
void *par_fft(void *var) {
    int i;
    double complex t;
    arg *arguments = var;
    double complex *buf = arguments->buf;
    double complex *out = arguments->out;
    int step = arguments->step;

    if (step < N) {
        full_seq_fft(out, buf, step * 2);
        full_seq_fft(out + step, buf + step, step * 2);
 
        for (i = 0; i < N; i += 2 * step) {
            t = cexp(-I * PI * i / N) * out[i + step];
            buf[i / 2] = out[i] + t;
            buf[(i + N) / 2] = out[i] - t;
        }
    }

    return NULL;
}

/**
 * Functia de rulat secvential cu posibilitate de paralelizare
 * Transformata Fourier Rapida
 */
void seq_fft(double complex *buf, double complex *out, int step) {
    int i;
    double complex t;
    pthread_t fork1, fork2;
    arg arguments1;
    arg arguments2;

    if (step < N) {
        if (step == numThreads / 2) {
            arguments1.buf = out;
            arguments1.out = buf;
            arguments1.step = step * 2;
            pthread_create(&fork1, NULL, par_fft, &arguments1);

            arguments2.buf = out + step;
            arguments2.out = buf + step;
            arguments2.step = step * 2;
            pthread_create(&fork2, NULL, par_fft, &arguments2);

            pthread_join(fork1, NULL);
            pthread_join(fork2, NULL);
        } else {
            seq_fft(out, buf, step * 2);
            seq_fft(out + step, buf + step, step * 2);
        }
 
        for (i = 0; i < N; i += 2 * step) {
            t = cexp(-I * PI * i / N) * out[i + step];
            buf[i / 2] = out[i] + t;
            buf[(i + N) / 2] = out[i] - t;
        }
    }
}

int main(int argc, char **argv) {
    getArgs(argc, argv);
    init();

    seq_fft(values, helper, 1);

    show();
    clean();
    return 0;
}
