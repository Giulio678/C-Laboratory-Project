#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

/* Controlla se il risultato (s) è -1; se vero, stampa l'errore, stampa errno e termina l'esecuzione */
#define CHECK_ERROR_NEGATIVE(s,m) \
    if ((s) == -1) { \
        perror(m); \
        fprintf(stderr, "Error errno: %d\n", errno); \
        exit(EXIT_FAILURE); \
    }

/* Controlla se il risultato (s) è diverso da zero; se vero, stampa l'errore, stampa errno e termina l'esecuzione */
#define CHECK_ERROR_ZERO(s,m) \
    if ((s) != 0) { \
        perror(m); \
        fprintf(stderr, "Error errno: %d\n", errno); \
        exit(EXIT_FAILURE); \
    }

/* Controlla se il risultato (s) è diverso da zero e diverso da ETIMEDOUT; 
   se vero, stampa l'errore, stampa errno e termina l'esecuzione */
#define CHECK_ERROR_TIMEDWAIT(s,m) \
    if ((s) != 0 && s != ETIMEDOUT) { \
        perror(m); \
        fprintf(stderr, "Error errno: %d\n", errno); \
        exit(EXIT_FAILURE); \
    }

/* Controlla se il risultato (s) è diverso da zero e diverso da EBUSY; 
   se vero, stampa l'errore, stampa errno e termina l'esecuzione */
#define CHECK_ERROR_TRYLOCK(s,m) \
    if ((s) != 0 && s != EBUSY) { \
        perror(m); \
        fprintf(stderr, "Error errno: %d\n", errno); \
        exit(EXIT_FAILURE); \
    }

/* Controlla se il risultato (s) è -1; se vero, stampa l'errore, stampa errno, esegue c (solitamente una funzione di cleanup) e termina l'esecuzione */
#define CHECK_ERROR_DESCRIPTOR(s,m,c) \
    if ((s) == -1) { \
        perror(m); \
        fprintf(stderr, "Error errno: %d\n", errno); \
        c; \
        exit(EXIT_FAILURE); \
    }

/* Controlla se il risultato (s) è NULL; se vero, stampa l'errore e termina l'esecuzione */
#define CHECK_ERROR_NULL(s,m) \
    if ((s) == NULL) { \
        perror(m); \
        exit(EXIT_FAILURE); \
    }

/* Controlla se il risultato (s) è EOF; se vero, stampa l'errore e termina l'esecuzione */
#define CHECK_ERROR_EOF(s,m) \
    if ((s) == EOF) { \
        perror(m); \
        exit(EXIT_FAILURE); \
    }

/* Alloca memoria usando malloc, con controllo dell'errore */
void *Malloc(size_t size,int n);

/* Alloca memoria e la inizializza a zero usando calloc, con controllo dell'errore */
void *Calloc(size_t data,size_t size);

/* Apre un file con fopen, con controllo dell'errore */
void Fopen(const char *filename, const char *mode, FILE **file);

/* Apre un file con fopen e restituisce il valore di ritorno*/
void *Fopen2(const char *filename, const char *mode, FILE **file);

/* Libera la memoria precedentemente allocata con malloc o calloc */
void myFree(void *ptr);

/* Chiude un file con fclose, con controllo dell'errore */
void Fclose(FILE **file);

/* Chiude un descrittore di file con close, con controllo dell'errore */
void Close(int server);

/* Funzione di cleanup, utilizzata per liberare risorse in caso di errore */
void cleanup(char *p, int fd);

#endif
