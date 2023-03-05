#include <unistd.h> //std lib
#include <stdio.h>
#include <math.h>
#include <stdlib.h> //exit, EXIT_FAILURE
#include <string.h> //strtok, atoi
#include <stdbool.h>
#include <omp.h>

#define INFILE "numeros.csv"
#define OUTFILE "ordenados.csv"

void par_qsort(int *data, int lo, int hi);

int main(int argc, char *argv[])
{

    int N = atoi(argv[1]);

    int *x = malloc(N * sizeof(int));
    int j;

    int primeNumber;
    char num;

    int posibles_elementos = N / 2;
    #pragma omp parallel for shared(x) private(j)
    for (j = 0; j < N; j++)
        x[j] = rand() % (posibles_elementos) + 1;


    double start_time = omp_get_wtime();

#pragma omp parallel
    {
#pragma omp single nowait
        par_qsort(x, 0, N - 1);
    }

    double end_time = omp_get_wtime();

    printf("Primeros Elementos: %d, %d, %d\n", x[0], x[1], x[2]);
    printf("Medios Elementos: %d, %d, %d\n", x[N / 4], x[N / 4 + 1], x[N / 4 + 2]);
    printf("Ultimos Elementos: %d, %d, %d\n", x[N - 3], x[N - 2], x[N - 1]);
    printf("Tiempo de ejecucion: %f segundos\n", end_time - start_time);

    FILE *escribirNumeros = fopen(INFILE, "w");
    if (escribirNumeros == NULL)
    {
        fprintf(stderr, "Falló la creación del archivo %s\n", INFILE);
        exit(EXIT_FAILURE);
    }

    // Llenamos el archivo INFILE con números
    #pragma omp parallel shared(escribirNumeros) private(j)
    {
        #pragma omp for
        for (j = 0; j < (N - 1); j++)
        {
            fprintf(escribirNumeros, "%d,", x[j]);
        }
    }
    fclose(escribirNumeros); // Cerramos el archivo si no lo vamos a usar nuevamente

    // Constructores para lectura de archivo y escritura de resultados
    // Esta es otra forma de realizar el manejo de arhivos
    FILE *leerNumeros = fopen(INFILE, "r");
    if (leerNumeros == NULL)
    {
        fprintf(stderr, "No se pudo leer el archivo %s\n", INFILE);
        exit(EXIT_FAILURE);
    }

    FILE *escribirOrdenados = fopen(OUTFILE, "w");
    if (escribirOrdenados == NULL)
    {
        fprintf(stderr, "No se pudo crear el archivo %s\n", OUTFILE);
        exit(EXIT_FAILURE);
    }

    // Leemos el archivo INFILE y escribimos los resultados en OUTFILE
    char *ch = (char *)malloc(256 * sizeof(char));
    #pragma omp parallel shared(leerNumeros, escribirOrdenados) private(ch)
    {
        #pragma omp while (fgets(ch, 256, leerNumeros) != NULL)
        {
            char *token = strtok(ch, ",");
            while (token != NULL)
            {
                printf("%s ", token);
                fprintf(escribirOrdenados, "%s ", token);
                token = strtok(NULL, ",");
            }
        }
    }
    printf("\n");

    free(ch);
    fclose(leerNumeros);
    fclose(escribirOrdenados);

    return 0;
}

void par_qsort(int *data, int lo, int hi)
{
    if (lo >= hi)
        return;

    int l = lo;
    int h = hi;
    int p = data[(hi + lo) / 2];

    while (l <= h)
    {
        while (data[l] < p)
            l++;
        while (data[h] > p)
            h--;
        if (l <= h)
        {
            int tmp = data[l];
            data[l] = data[h];
            data[h] = tmp;
            l++;
            h--;
        }
    }

#pragma omp task
    par_qsort(data, lo, h);

#pragma omp task
    par_qsort(data, l, hi);
}
